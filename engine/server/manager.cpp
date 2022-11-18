/*
 * Ham World Engine Server Manager
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

#include "ham/check.h"
#include "ham/fs.h"
#include "ham/str_buffer.h"

#include "manager.h"

#include <sys/wait.h>
#include <alloca.h>
#include <fcntl.h>
#include <signal.h>
#include <getopt.h>
#include <ncurses.h>

HAM_C_API_BEGIN

constexpr char ham_engine_server_manager_version_str[] = "Ham World Engine Server Manager - " HAM_ENGINE_BUILD_TYPE_STR " - v" HAM_ENGINE_VERSION_STR;

constexpr char ham_engine_server_manager_version_fancy_str[] =
	"┌──┐\n"
	"│██│    ┌──┐\n"
	"│██│    │██│ ┌──────┐   ┌───┐  ┌───┐\n"
	"│██└────┘██├─┘██████├──┬┘███└──┘███└┐\n"
	"│██████████│██┌─────┤██│████████████│\n"
	"│██┌────┐██│██│     │██│██┌─┐██┌─┐██│\n"
	"│██│    │██│██└─────┘██│██│ └──┘ │██│\n"
	"│██│    │██├─┐█████████│██│      │██│\n"
	"└──┘    └──┘ └─────────┴──┘      └──┘ World Engine Server Manager - " HAM_ENGINE_BUILD_TYPE_STR " - v" HAM_ENGINE_VERSION_STR
;

struct ham_engine_server_manager{
	const ham_allocator *allocator;

	ham::str_buffer8 server_exec_path;
	int server_argc;
	char **server_argv;

	int server_fds[2];

	WINDOW *nc_win, *input_win, *log_win;
	int w, h;

	ham::str_buffer8 input_line;
	ham::str_buffer8 log_buf;

	ham_f64 startup_delay = 1.0;
	ham_f64 startup_counter = 0.0;

	ham_f64 shutdown_timeout = 10.0;
	ham_f64 shutdown_counter = 0.0;

	bool shutdown_flag = false;
	bool wants_exit = false;
	bool autostart_flag = true;

	pid_t server_pid = (pid_t)-1;
};

ham_engine_server_manager *ham_server_manager_create(const char *server_exec_path, int server_argc, char **server_argv){
	if(!ham_check(server_exec_path != NULL)){
		return nullptr;
	}
	else if(stdscr){
		ham_logapierrorf("Only a single server manager instance may be created per process group");
		return nullptr;
	}

	ham_file_info info;
	if(!ham_path_file_info_utf8(ham::str8(server_exec_path), &info)){
		ham_logapierrorf("Failed to get file info for path: %s", server_exec_path);
		return nullptr;
	}
	else if(info.kind != HAM_FILE_REGULAR){
		ham_logapierrorf("Given path is not a regular file: %s", server_exec_path);
		return nullptr;
	}

	int server_fds[2];
	int res = pipe(server_fds);
	if(res != 0){
		ham_logapierrorf("Error in pipe: %s", strerror(errno));
		return nullptr;
	}

	res = fcntl(server_fds[0], F_SETFL, O_NONBLOCK);
	if(res == -1){
		ham_logapierrorf("Error in fcntl: %s", strerror(errno));
		close(server_fds[0]);
		close(server_fds[1]);
		return nullptr;
	}

	WINDOW *const nc_win = initscr();
	if(!nc_win){
		ham_logapierrorf("Error in initscr");
		close(server_fds[0]);
		close(server_fds[1]);
		return nullptr;
	}

	start_color();

	const auto allocator = ham_current_allocator();

	const auto ptr = ham_allocator_new(allocator, ham_engine_server_manager);
	if(!ptr){
		endwin();
		close(server_fds[0]);
		close(server_fds[1]);
		return nullptr;
	}

	ptr->server_exec_path = server_exec_path;
	ptr->server_argc = server_argc;
	ptr->server_argv = server_argv;

	memcpy(ptr->server_fds, server_fds, sizeof(server_fds));

	ptr->nc_win = nc_win;

	getmaxyx(nc_win, ptr->h, ptr->w);

	ptr->log_win = newwin(ptr->h - 4, (2 * ptr->w) / 3, 1, 1);
	ptr->input_win = newwin(1, ptr->w - 2, ptr->h - 2, 1);

	return ptr;
}

void ham_server_manager_destroy(ham_engine_server_manager *manager){
	if(!manager) return;

	const auto allocator = manager->allocator;

	if(manager->server_pid != (pid_t)-1){
		kill(manager->server_pid, SIGTERM);
	}

	close(manager->server_fds[0]);
	close(manager->server_fds[1]);

	if(endwin() != 0){
		ham_logapiwarnf("Error in endwin");
	}

	ham_allocator_delete(allocator, manager);
}


#define HAM_NCURSES_COLOR_BACK      234 // Grey11       #1c1c1c
#define HAM_NCURSES_COLOR_HIGHLIGHT 208 // DarkOrange   #ff8700
#define HAM_NCURSES_COLOR_TEXT      231 // Grey100      #ffffff
#define HAM_NCURSES_COLOR_BORDER    253 // Grey85       #dadada
#define HAM_NCURSES_COLOR_GREEN     40  // Green3       #00d700
#define HAM_NCURSES_COLOR_RED       124 // Red3         #af0000
#define HAM_NCURSES_COLOR_YELLOW    187 // LightYellow3 #af0000

#define HAM_NCURSES_PAIR_REGULAR      1
#define HAM_NCURSES_PAIR_HIGHLIGHT    2
#define HAM_NCURSES_PAIR_BORDER       3
#define HAM_NCURSES_PAIR_STATUS_GOOD  4
#define HAM_NCURSES_PAIR_STATUS_ERROR 5
#define HAM_NCURSES_PAIR_STATUS_WARN  6

void ham_server_start_server_exec(ham_engine_server_manager *manager){
	if(manager->server_pid != (pid_t)-1) return;

	const pid_t fork_res = fork();
	if(fork_res == 0){
		// child process

		// redirect stdout/stderr
		dup2(manager->server_fds[1], STDOUT_FILENO);
		dup2(manager->server_fds[1], STDERR_FILENO);

		// Child doesn't read
		//close(manager->server_fds[0]);

		// not gonna be able to free this, gotta use alloca
		const auto new_args = (char**)alloca(sizeof(void*) * (manager->server_argc + 2));

		memcpy(&new_args[1], &manager->server_argv[0], sizeof(void*) * manager->server_argc);
		new_args[0] = const_cast<char*>(manager->server_exec_path.c_str()); // fuckin argv
		new_args[manager->server_argc + 1] = nullptr;

		if(execv(manager->server_exec_path.c_str(), new_args) == -1){
			// error *executing* server executable
			ham_logapierrorf("Error in execv: %s", strerror(errno));
			exit(EXIT_FAILURE);
		}
	}
	else if(fork_res == -1){
		// error in fork
		ham_logapierrorf("Error in fork: %s", strerror(errno));
		return;
	}

	manager->server_pid = fork_res;
}

int ham_server_manager_exec(ham_engine_server_manager *manager){
	if(!ham_check(manager != NULL)) return false;

	ham_ticker ticker;
	ham_ticker_reset(&ticker);

	if(has_colors()){
		init_pair(HAM_NCURSES_PAIR_REGULAR,      HAM_NCURSES_COLOR_TEXT,      HAM_NCURSES_COLOR_BACK);
		init_pair(HAM_NCURSES_PAIR_HIGHLIGHT,    HAM_NCURSES_COLOR_HIGHLIGHT, HAM_NCURSES_COLOR_BACK);
		init_pair(HAM_NCURSES_PAIR_BORDER,       HAM_NCURSES_COLOR_BORDER,    HAM_NCURSES_COLOR_BACK);
		init_pair(HAM_NCURSES_PAIR_STATUS_GOOD,  HAM_NCURSES_COLOR_GREEN,     HAM_NCURSES_COLOR_BACK);
		init_pair(HAM_NCURSES_PAIR_STATUS_ERROR, HAM_NCURSES_COLOR_RED,       HAM_NCURSES_COLOR_BACK);
		init_pair(HAM_NCURSES_PAIR_STATUS_WARN,  HAM_NCURSES_COLOR_YELLOW,    HAM_NCURSES_COLOR_BACK);

		wbkgd(manager->nc_win, COLOR_PAIR(HAM_NCURSES_PAIR_BORDER));
	}

	curs_set(0);

	cbreak();
	nodelay(manager->input_win, true);

	ham_message_buffer_utf8 message_buf;

	while(true){
		const ham_f64 dt = ham_ticker_tick(&ticker, 1.0/20.0);
		(void)dt;

		if(manager->server_pid != (pid_t)-1){
			int status;
			int wait_res = waitpid(manager->server_pid, &status, WNOHANG);
			if(wait_res == manager->server_pid){
				// TODO: check status
				manager->server_pid = -1;
				manager->shutdown_flag = false;
				manager->shutdown_counter = 0.0;
			}
			else if(wait_res == -1){
				ham_logapierrorf("Error in waitpid: %s", strerror(errno));
			}
			else if(wait_res == 0){
				// child is good, we should process input

				const ssize_t read_res = read(manager->server_fds[0], message_buf, sizeof(message_buf)-1);
				if(read_res > 0){
					manager->log_buf.append(ham::str8(message_buf, read_res));
				}
				else if(errno != EAGAIN && errno != EWOULDBLOCK){
					ham_logapierrorf("Error in read: %s", strerror(errno));
					kill(manager->server_pid, SIGKILL);
					manager->server_pid = (pid_t)-1;
				}
			}

			if(manager->shutdown_flag){
				manager->shutdown_counter += dt;

				wait_res = waitpid(manager->server_pid, &status, WNOHANG);
				if(
					(wait_res == 0 && manager->shutdown_counter > manager->shutdown_timeout) ||
					(wait_res == -1)
				){
					kill(manager->server_pid, SIGKILL);
					manager->server_pid = (pid_t)-1;
					manager->shutdown_flag = false;
				}
			}
		}
		else if(!manager->wants_exit && manager->autostart_flag){
			manager->startup_counter += dt;
			if(manager->startup_counter >= manager->startup_delay){
				ham_server_start_server_exec(manager);
				manager->startup_counter = 0.0;
			}
		}
		else{ // manager->wants_exit && manager->server_pid == (pid_t)-1
			break;
		}

		int new_h, new_w;
		getmaxyx(manager->nc_win, new_h, new_w);

		if(new_h != manager->h || new_w != manager->w){
			mvwin(manager->input_win, new_h - 2, 1);
			wresize(manager->log_win, (2 * new_w) / 3, new_h - 4);
			wresize(manager->input_win, 1, new_w - 2);
			manager->h = new_h;
			manager->w = new_w;
		}

		ham_server_manager_redraw(manager);

		int ch = wgetch(manager->input_win);

		switch(ch){
			case KEY_CLOSE: ham_server_manager_exit(manager); break;

			case KEY_BACKSPACE:
			case KEY_DC:
			case 127:{
				const auto old_len = manager->input_line.len();
				if(old_len > 0){
					manager->input_line.resize(old_len-1);
				}

				break;
			}

			default:{
				if(isprint(ch)){
					// TODO: process user input
					const char chstr[] = { (char)ch, '\0' };
					manager->input_line.append(chstr);
				}

				break;
			}
		}
	}

	nocbreak();

	return 0;
}

bool ham_server_manager_redraw_border(ham_engine_server_manager *manager){
	if(!ham_check(manager != NULL)) return false;

	//
	// Borders and dividers
	//

	// main border
	wattron(manager->nc_win, COLOR_PAIR(HAM_NCURSES_PAIR_BORDER));
	wborder(
		manager->nc_win,
		ACS_VLINE, // left side
		ACS_VLINE, // right side
		ACS_HLINE, // top side
		ACS_HLINE, // bottom side
		ACS_ULCORNER, // top left
		ACS_URCORNER, // top right
		ACS_LLCORNER, // bottom left
		ACS_LRCORNER  // bottom right
	);
	wattroff(manager->nc_win, COLOR_PAIR(HAM_NCURSES_PAIR_BORDER));

	// border around user input
	wattron(manager->nc_win, COLOR_PAIR(HAM_NCURSES_PAIR_BORDER));
	wmove(manager->nc_win, manager->h - 3, 0);
	waddch(manager->nc_win, ACS_LTEE);
	whline(manager->nc_win, ACS_HLINE, manager->w - 2);
	wmove(manager->nc_win, manager->h - 3, manager->w - 1);
	waddch(manager->nc_win, ACS_RTEE);
	wattroff(manager->nc_win, COLOR_PAIR(HAM_NCURSES_PAIR_BORDER));

	//
	// Inline information in border
	//

	wattron(manager->nc_win, COLOR_PAIR(HAM_NCURSES_PAIR_HIGHLIGHT));
	mvwprintw(manager->nc_win, 0, 2, " Ham Server Manager v" HAM_ENGINE_VERSION_STR " ");
	wattroff(manager->nc_win, COLOR_PAIR(HAM_NCURSES_PAIR_HIGHLIGHT));

	if(manager->server_pid != (pid_t)-1){
		wattron(manager->nc_win, COLOR_PAIR(HAM_NCURSES_PAIR_STATUS_GOOD));
		mvwprintw(manager->nc_win, 0, manager->w - 5, " ⬤ ");
		wattroff(manager->nc_win, COLOR_PAIR(HAM_NCURSES_PAIR_STATUS_GOOD));
	}
	else{
		// TODO: check for warning status
		wattron(manager->nc_win, COLOR_PAIR(HAM_NCURSES_PAIR_STATUS_ERROR));
		mvwprintw(manager->nc_win, 0, manager->w - 5, " ⬤ ");
		wattroff(manager->nc_win, COLOR_PAIR(HAM_NCURSES_PAIR_STATUS_ERROR));
	}

	return true;
}

bool ham_server_manager_redraw(ham_engine_server_manager *manager){
	if(!ham_check(manager != NULL)) return false;

	// TODO: damage tracking
	wattron(manager->nc_win, COLOR_PAIR(HAM_NCURSES_PAIR_BORDER));
	werase(manager->nc_win);
	wattroff(manager->nc_win, COLOR_PAIR(HAM_NCURSES_PAIR_BORDER));

	ham_server_manager_redraw_border(manager);

	wrefresh(manager->nc_win);

	//
	// Server logs
	//

	const auto log_buf_str = manager->log_buf.c_str();
	if(log_buf_str){
		wattron(manager->nc_win, COLOR_PAIR(HAM_NCURSES_PAIR_STATUS_GOOD));
		mvwprintw(manager->log_win, 0, 0, "%s", log_buf_str);
		wattroff(manager->nc_win, COLOR_PAIR(HAM_NCURSES_PAIR_STATUS_GOOD));
		//wrefresh(manager->log_win);
	}

	//
	// User input
	//

	wmove(manager->input_win, 0, 0);

	const auto input_line_str = manager->input_line.c_str();
	if(input_line_str){
		wprintw(manager->input_win, "> %s", input_line_str);
	}
	else{
		wprintw(manager->input_win, "> ");
	}

	return true;
}

bool ham_server_manager_exit(ham_engine_server_manager *manager){
	if(!ham_check(manager != NULL)) return false;

	manager->wants_exit = true;
	manager->shutdown_flag = true;
	manager->shutdown_counter = 0.0;

	return true;
}

bool ham_server_manager_shutdown(ham_engine_server_manager *manager){
	if(!ham_check(manager != NULL)) return false;
	else if(manager->shutdown_flag || manager->server_pid == (pid_t)-1) return true;

	manager->shutdown_flag = true;
	manager->shutdown_counter = 0.0;

	return true;
}

HAM_C_API_END

namespace ham::engine{
	class server_manager_options{
		public:
			const char *server_exec_path;
			int server_argc = 0;
			char **server_argv = nullptr;
	};
}

static inline const char *ham_impl_manager_version_str(bool fancy){
	(void)fancy; // not used... yet

	return fancy
		? ham_engine_server_manager_version_fancy_str
		: ham_engine_server_manager_version_str
	;
}

[[noreturn]]
static void ham_impl_show_version(bool fancy = false){
	printf("%s\n", ham_impl_manager_version_str(fancy));
	exit(EXIT_SUCCESS);
}

[[noreturn]]
static void ham_impl_show_help(const char *argv0){
	const char *exec_name = argv0;

	const auto argv0_str = ham::str8(argv0);
	const auto new_beg = argv0_str.rfind("/");
	if(new_beg != ham::str8::npos){
		exec_name += new_beg + 1;
	}

	printf(
		"%s\n"
		"\n"
		"    Usage: %s [OPTIONS]\n"
		"\n"
		"    Possible options:\n"
		"        -h|--help:             Print this help message and exit\n"
		"        -v|--version:          Print the version string and exit\n"
		"        -e|--server-exec PATH:  Set server executable path\n"
		"\n"
		,
		ham_impl_manager_version_str(true),
		exec_name
	);

	exit(EXIT_SUCCESS);
}

static struct option ham_impl_long_options[] = {
	{ "help",        no_argument,       nullptr, 0 },
	{ "version",     no_argument,       nullptr, 0 },
	{ "server-exec", required_argument, nullptr, 0 },
	{ nullptr,       0,                 nullptr, 0 },
};

int ham_engine_server_manager_parse_args(ham::engine::server_manager_options &options, int argc, char *argv[]){
	while(1){
		int option_index = 0;
		const int getopt_res = getopt_long(argc, argv, "hve:", ham_impl_long_options, &option_index);

		if(getopt_res == -1){
			break;
		}

		switch(getopt_res){
			case 0:{
				switch(option_index){
					case 0: ham_impl_show_help(argv[0]);
					case 1: ham_impl_show_version();

					case 2:{
						options.server_exec_path = optarg;
						break;
					}

					default:{
						fprintf(stderr, "Error in getopt_long: unexpected long option at index %d\n", option_index);
						exit(1);
					}
				}

				break;
			}

			case 'h': ham_impl_show_help(argv[0]);
			case 'v': ham_impl_show_version();

			case 'e':{
				options.server_exec_path = optarg;
				break;
			}

			case '?':{
				fprintf(stderr, "Unexpected argument: %s\n", optarg);
				exit(1);
			}

			default:{
				fprintf(stderr, "Error in getopt_long: returned character 0x%o\n", getopt_res);
				exit(1);
			}
		}
	}

	if(optind < argc){
		options.server_argc = argc - optind;
		options.server_argv = argv + optind;
	}

	return 0;
}

int main(int argc, char *argv[]){
	setlocale(LC_ALL, "");

	constexpr ham::str8 default_exec_stem = "ham-engine-server" HAM_PLATFORM_EXEC_EXT;

	ham_path_buffer_utf8 path_buf;
	const char *pwd_ = getcwd(path_buf, HAM_PATH_BUFFER_SIZE);
	const ham_usize pwd_len = strlen(pwd_);

	constexpr ham_usize max_pwd_len = HAM_PATH_BUFFER_SIZE - (1 + default_exec_stem.len());

	if(pwd_len >= max_pwd_len){
		fprintf(stderr, "[INTERNAL ERROR] pwd too long: %zu, max %zu", pwd_len, max_pwd_len);
		return 69;
	}

	const ham_usize default_exec_len = pwd_len + default_exec_stem.len() + 1;

	path_buf[pwd_len] = '/';
	memcpy(path_buf + pwd_len + 1, default_exec_stem.ptr(), default_exec_stem.len());
	path_buf[default_exec_len + 1] = '\0';

	ham::engine::server_manager_options options;
	options.server_exec_path = path_buf;

	int res = ham_engine_server_manager_parse_args(options, argc, argv);
	if(res != 0){
		fprintf(stderr, "Error parsing arguments\n");
		return res;
	}

	const auto server_manager = ham_server_manager_create(options.server_exec_path, options.server_argc, options.server_argv);
	if(!server_manager){
		fprintf(stderr, "Error in ham_server_manager_create");
		return 1;
	}

	res = ham_server_manager_exec(server_manager);

	ham_server_manager_destroy(server_manager);

	return res;
}
