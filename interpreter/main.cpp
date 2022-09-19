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
