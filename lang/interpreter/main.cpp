/*
 * Ham Programming Language Interpreter
 * Copyright (C) 2022 Hamsmith Ltd.
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

#include "ham/typedefs.h"
#include "ham/std_vector.hpp"

#include <stdlib.h>
#include <stdio.h>

#include "readline/readline.h"
#include "readline/history.h"

static ham::std_vector<char*> alloced_strs;

static ham::str8 hami_readline(const char *prompt){
	char *user_input = readline(prompt);
	if(!user_input) return {};

	if(!*user_input){
		return ham::str8(user_input);
	}

	add_history(user_input);
	alloced_strs.emplace_back(user_input);
	return ham::str8(user_input);
}

int main(int argc, char *argv[]){
	(void)argc; (void)argv;

	atexit([]{
		for(auto alloced_str : alloced_strs){
			free(alloced_str);
		}
	});

	ham::str8 user_input;

	while(true){
		user_input = hami_readline("> ");
		if(!user_input.ptr() || !user_input[0]){
			continue;
		}

		if(user_input == "exit" || user_input == "quit"){
			break;
		}
	}

	return 0;
}
