/**
 * Ham Programming Language Compiler
 * Copyright (C) 2022 Keith Hammond
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "ham/fs.h"
#include "ham/str_buffer.h"
#include "ham/parse.h"

#include "ham/std_vector.hpp"

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>

using namespace ham::typedefs;

namespace ham{
	struct compiler_options{
		bool verbose = false;
		bool quiet = false;
		str8 output_file = "hamc-out";
		std_vector<str_buffer_utf8> input_files;
	};
}

[[noreturn]]
static inline void display_version(){
	printf("Ham Compiler v" HAM_VERSION_STR "\n");
	exit(EXIT_SUCCESS);
}

[[noreturn]]
static inline void display_help(const char *argv0){
	printf(
		"Ham Compiler v" HAM_VERSION_STR "\n"
		"\n"
		"hamc  Copyright (C) 2022 Keith Hammond\n"
		"This program comes with ABSOLUTELY NO WARRANTY.\n"
		"This is free software, and you are welcome to redistribute it\n"
		"under the conditions of the GNU General Public License version 3 or later.\n"
		"\n"
		"    Usage: hamc [OPTION...] input-files\n"
		"\n"
		"    Possible 'OPTION's:\n"
		"\n"
		"        -h|--help               Display this message.\n"
		"        -Q|--quiet              Don't output any diagnostic or build messages.\n"
		"        -V|--verbose            Output all diagnostic messages.\n"
		"        -o|--output-file file   File path for output of compilation.\n"
		"\n"
	);

	exit(EXIT_SUCCESS);
}

int parse_options(ham::compiler_options *comp_options, int argc, char *argv[]){
	const struct option long_options[] = {
		// none of our options set flags, as we don't tend to use straight up 'int's
		{"help",        no_argument,       nullptr, 'h'},
		{"version",     no_argument,       nullptr, 0  },

		{"quiet",       no_argument,       nullptr, 'Q'},
		{"verbose",     no_argument,       nullptr, 'V'},
		{"output-file", required_argument, nullptr, 'o'},

		// end marker
		{nullptr,   0,                 nullptr, 0 }
	};

	while(true){
		int option_index = 0;
		const int res = getopt_long(argc, argv, "ho:VQ", long_options, &option_index);
		if(res == -1) break;

		switch(res){
			// long option
			case 0:{
				// this option set a flag, which is unlikely. continue on soldier
				if(long_options[option_index].flag != nullptr){
					break;
				}

				switch(option_index){
					case 0: display_help(argv[0]);
					case 1: display_version();

					case 2:{ // quiet
						comp_options->quiet = true;
						break;
					}

					case 3:{ // verbose
						comp_options->verbose = true;
						break;
					}

					case 4:{ // output-file
						comp_options->output_file = ham::str8(optarg);
						break;
					}

					default: abort();
				}

				break;
			}

			case 'h': display_help(argv[0]);
			case 'Q': comp_options->quiet = true; break;
			case 'V': comp_options->verbose = true; break;
			case 'o': comp_options->output_file = ham::str8(optarg); break;

			default: abort();
		}
	}

	while(optind < argc){
		comp_options->input_files.emplace_back((const char*)argv[optind++]);
	}

	return 0;
}

int main(int argc, char *argv[]){
	ham::compiler_options comp_options;
	if(parse_options(&comp_options, argc, argv) != 0){
		return 1;
	}

	if(comp_options.input_files.empty()){
		if(!comp_options.quiet){
			fprintf(stderr, "No input files given.\n");
		}
		return 2;
	}

	ham::std_vector<ham::parse_context_utf8> ctxs;

	for(auto &&path_buf : comp_options.input_files){
		const auto input_file = ham::file(path_buf.get(), ham::file_open_flags::read);
		if(!input_file){
			if(!comp_options.quiet) fprintf(stderr, "Could not open input file: %s\n", path_buf.c_str());
			return 3;
		}

		const usize file_size = input_file.size();
		if(file_size == (usize)-1){
			if(!comp_options.quiet) fprintf(stderr, "Could not get file size: %s\n", path_buf.c_str());
			return 3;
		}
		else if(file_size == 0){
			// skip empty files
			continue;
		}

		// TODO: check file encoding
		// default to utf-8 for now

		ham::str_buffer_utf8 file_content_buf;
		if(!file_content_buf.resize(file_size-1)){
			if(!comp_options.quiet) fprintf(stderr, "Failed to allocate memory for file contents: %s\n", path_buf.c_str());
			return 3;
		}

		const usize read_res = input_file.read(file_content_buf.ptr(), file_size-1);
		if(read_res == (usize)-1){
			if(!comp_options.quiet) fprintf(stderr, "Failed to read file contents: %s\n", path_buf.c_str());
			return 3;
		}

		ham::parse_context_utf8 ctx;
		if(!ctx){
			if(!comp_options.quiet) fprintf(stderr, "Failed to create parse context: %s\n", path_buf.c_str());
			return 3;
		}

		//
		// Lexing
		//

		ham::std_vector<ham::token_utf8> toks;

		ham::source_location_utf8 loc;
		ham::token_utf8 tok;
		ham::usize off = 0;

		while(ham::lex(loc, ham::str8(file_content_buf.ptr() + off, file_content_buf.len() - off), tok)){
			toks.emplace_back(tok);
			off += tok.str().len();
		}

		if(tok.kind() == ham::token_kind::error){
			if(!comp_options.quiet) fprintf(stderr, "Lexing error[%s]: %s\n", path_buf.c_str(), tok.str().ptr());
			return 4;
		}

		//
		// Parsing
		//

		const auto toks_beg = toks.begin();
		const auto toks_end = toks.end();
		auto toks_it = toks_beg;

		ham::expr_base_utf8 prev_expr;

		while(toks_it != toks_end && !toks_it->is_eof()){
			const auto parse_res = ham::parse(ctx, ham::basic_token_range{ &*toks_it, &*toks_end });
			if(!parse_res){
				if(!comp_options.quiet) fprintf(stderr, "Parsing error [%s]\n", path_buf.c_str());
				return 5;
			}
			else if(parse_res.is_error()){
				const ham::expr_error_utf8 err_expr = (const ham_expr_error_utf8*)parse_res.handle();

				const auto err_toks = err_expr.tokens();
				const auto err_toks_beg = err_toks.begin();
				const auto err_toks_end = err_toks.end();

				ham::source_location_utf8 src_loc{path_buf.get(), 0, 0};

				if(err_toks_beg != err_toks_end){
					src_loc = err_toks_beg->source_location();
				}

				if(!comp_options.quiet){
					fprintf(
						stderr,
						"Parsing error [%s:%zu:%zu]: %s\n",
						path_buf.c_str(), src_loc.line() + 1, src_loc.column() + 1,
						err_expr.message().ptr()
					);
				}
				return 5;
			}

			prev_expr = parse_res;
		}

		ctxs.emplace_back(std::move(ctx));
	}

	// TODO: resolve/combine the damn contexts

	// TODO: typecheck the god-damn thing

	// TODO: compile the bastard

	// TODO: output 'em to a file or possibly stdout in future

	return 0;
}
