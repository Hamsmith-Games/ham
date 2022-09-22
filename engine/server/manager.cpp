#include "ham/check.h"
#include "ham/fs.h"
#include "ham/str_buffer.h"
#include "ham/std_vector.hpp"

#include "manager.h"

#include <sys/wait.h>
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

	ham::str_buffer_utf8 server_exec_path;
	int server_argc;
	char **server_argv;

	int server_fds[2];

	WINDOW *nc_win;
	int w, h;

	ham::str_buffer_utf8 input_line;

	ham_f64 shutdown_timeout = 10.0;
	ham_f64 shutdown_counter = 0.0;

	bool shutdown_flag = false;
	bool autostart_flag = false;
	bool wants_exit = false;

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
	const int res = pipe(server_fds);
	if(res != 0){
		ham_logapierrorf("Error in pipe: %s", strerror(errno));
		return nullptr;
	}

	WINDOW *const nc_win = initscr();
	if(!nc_win){
		ham_logapierrorf("Error in initscr");
		return nullptr;
	}

	start_color();

	const auto allocator = ham_current_allocator();

	const auto ptr = ham_allocator_new(allocator, ham_engine_server_manager);
	if(!ptr){
		endwin();
		return nullptr;
	}

	ptr->server_exec_path = server_exec_path;
	ptr->server_argc = server_argc;
	ptr->server_argv = server_argv;

	memcpy(ptr->server_fds, server_fds, sizeof(server_fds));

	ptr->nc_win = nc_win;

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

	while(true){
		const ham_f64 dt = ham_ticker_tick(&ticker, 1.0/20.0);
		(void)dt;

		if(manager->shutdown_flag && manager->server_pid != (pid_t)-1){
			manager->shutdown_counter += dt;

			int status;
			const int wait_pid = waitpid(manager->server_pid, &status, WNOHANG);
			if(
				(wait_pid == 0 && manager->shutdown_counter > manager->shutdown_timeout) ||
				(wait_pid == -1)
			){
				kill(manager->server_pid, SIGKILL);
				manager->server_pid = (pid_t)-1;
				manager->shutdown_flag = false;
			}
		}
		else if(!manager->wants_exit && manager->autostart_flag && manager->server_pid == (pid_t)-1){
			// TODO: start server executable
		}

		if(manager->wants_exit && manager->server_pid == (pid_t)-1){
			break;
		}

		getmaxyx(manager->nc_win, manager->h, manager->w);

		ham_server_manager_redraw(manager);

		int ch = wgetch(manager->nc_win);
		if(isgraph(ch)){

		}

		switch(ch){
			//case 'q': ham_server_manager_exit(manager); break;

			default: break;
		}
	}

	nocbreak();

	return 0;
}

bool ham_server_manager_redraw_border(ham_engine_server_manager *manager){
	if(!ham_check(manager != NULL)) return false;

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
