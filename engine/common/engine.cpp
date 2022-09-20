/**
 * The Ham World Engine
 * Copyright (C) 2022  Hamsmith Ltd.
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

#include "ham/engine.h"
#include "ham/engine-vtable.h"
#include "ham/check.h"
#include "ham/fs.h"

#include <unistd.h>
#include <getopt.h>
#include <dirent.h>

HAM_C_API_BEGIN

[[noreturn]]
static inline void ham_impl_display_version(){
	printf("Ham World Engine v" HAM_VERSION_STR "\n");
	exit(EXIT_SUCCESS);
}

[[noreturn]]
static void ham_impl_print_help(const char *argv0){
	printf(
		"Ham World Engine v" HAM_VERSION_STR "\n"
		"\n"
		"    Usage: %s [OPTIONS]\n"
		"\n"
		"    Possible options:\n"
		"        -h|--help:          Print this help message\n"
		"        -g|--game-dir DIR:  Set the game/app directory\n"
		"\n"
		,
		argv0
	);
	exit(EXIT_SUCCESS);
}

struct ham_impl_engine_data{
	ham::str8 vtable_id;
	ham_dll_handle dll_handle = nullptr;
	const ham_engine_vtable *vtable = nullptr;
};

static inline bool ham_impl_find_engine_vtable(const ham_plugin *plugin, ham_impl_engine_data *data){
	const ham_usize num_vtables = ham_plugin_iterate_vtables(
		plugin,
		[](const ham_plugin_vtable *vtable, void *user){
			const auto data = reinterpret_cast<ham_impl_engine_data*>(user);

			if(vtable->category() != ham::str8(HAM_ENGINE_PLUGIN_CATEGORY)){
				return true;
			}

			if(
				(vtable->name()         == data->vtable_id) ||
				(vtable->display_name() == data->vtable_id)
			){
				data->vtable = (const ham_engine_vtable*)vtable;
				return false;
			}

			return true;
		},
		data
	);

	if(num_vtables == (ham_usize)-1){
		ham_logapiwarnf("Failed to iterate plugin vtables");
		return false;
	}

	return data->vtable != nullptr;
}

static inline const ham_engine_vtable *ham_impl_find_engine_plugin(ham::str8 path, ham_impl_engine_data *data){
	constexpr ham::str8 plugin_mime = HAM_PLATFORM_DLL_MIME;
	constexpr ham::str8 minimum_subpath = "/libx" HAM_PLATFORM_DLL_EXT;

	if((path.len() + minimum_subpath.len()) >= HAM_PATH_BUFFER_SIZE){
		ham_logerrorf("ham_engine_create", "Plugin dir too long: %zu, max %zu", path.len(), HAM_PATH_BUFFER_SIZE - (minimum_subpath.len() + 1));
		return nullptr;
	}

	ham_path_buffer_utf8 path_buf;
	memcpy(path_buf, path.ptr(), path.len());
	path_buf[path.len()] = '\0';

	DIR *plugin_dir = opendir(path_buf);
	if(!plugin_dir){
		ham_logerrorf("ham_engine_create", "Error in opendir: %s", strerror(errno));
		return nullptr;
	}

	path_buf[path.len()] = '/';

	char *plugin_name_subpath = path_buf + path.len() + 1;

	for(const struct dirent *plugin_dirent = readdir(plugin_dir); plugin_dirent != nullptr; plugin_dirent = readdir(plugin_dir)){
		if(plugin_dirent->d_type != DT_REG){
			continue;
		}

		ham::str8 plugin_name = plugin_dirent->d_name;

		if(path.len() + plugin_name.len() + 1 >= HAM_PATH_BUFFER_SIZE){
			ham_logapiwarnf("Plugin path too long: %zu, max %d", path.len() + plugin_name.len() + 1, HAM_PATH_BUFFER_SIZE-1);
			continue;
		}

		memcpy(plugin_name_subpath, plugin_name.ptr(), plugin_name.len());
		plugin_name_subpath[plugin_name.len()] = '\0';

		ham_file_info file_info;
		if(!ham_path_file_info_utf8(ham::str8((const char*)path_buf), &file_info)){
			ham_logapiwarnf("Could not get file info: %s", path_buf);
			continue;
		}
		else if(ham::str8(file_info.mime).substr(0, plugin_mime.len()) != plugin_mime){
			ham_logapiwarnf("Non-plugin found in plugins folder: %s", path_buf);
			continue;
		}

		const ham_dll_handle plugin_dll = ham_dll_open_c(plugin_dirent->d_name);
		if(!plugin_dll){
			ham_logapiwarnf("Failed to open plugin: %s", path_buf);
			continue;
		}

		ham_plugin *const plugin = ham_plugin_load(plugin_dll);
		if(!plugin){
			ham_logapiwarnf("Failed to load opened plugin: %s", path_buf);
			ham_dll_close(plugin_dll);
			continue;
		}

		const bool vtable_found = ham_impl_find_engine_vtable(plugin, data);

		ham_plugin_unload(plugin);

		if(vtable_found){
			data->dll_handle = plugin_dll;
			break;
		}
		else{
			ham_dll_close(plugin_dll);
		}
	}

	closedir(plugin_dir);
	return data->vtable;
}

static struct option ham_impl_long_options[] = {
	{ "help",		no_argument,		0, 0 },
	{ "game-dir",	required_argument,	0, 0 },
	{ 0,            0,                  0, 0 },
};

ham_uptr ham_impl_engine_thread_main(void *user){
	const auto ctx    = reinterpret_cast<ham_engine_context*>(user);
	const auto vtable = ctx->vtable;

	const auto engine_name = ham_super(vtable)->name();

	ham_logapidebugf("Posting engine semaphore...");

	if(!ham_sem_post(ctx->sem)){
		ham_logerrorf(engine_name.ptr, "Error posting engine semaphore");
		ctx->status = 1;
		return 1;
	}

	ham_logapidebugf("Waiting for engine mutex...");

	if(!ham_mutex_lock(ctx->mut)){
		ham_logerrorf(engine_name.ptr, "Error locking engine mutex");
		ctx->status = 1;
		return 1;
	}

	if(!ham_super(vtable)->on_load()){
		ham_logerrorf(engine_name.ptr, "Error loading plugin");
		ctx->status = 1;
		ham_mutex_unlock(ctx->mut);
		return 1;
	}

	ham_logapiinfof("Loaded %s", engine_name.ptr);

	if(!vtable->context_init(ctx)){
		ham_logerrorf(engine_name.ptr, "Error initializing context");

		ham_super(vtable)->on_unload();
		ham_logapiinfof("Unloaded %.*s", (int)engine_name.len, engine_name.ptr);

		ctx->status = 1;
		ham_mutex_unlock(ctx->mut);
		return 1;
	}

	ham_logapidebugf("Waiting for engine semaphore posts...");

	if(!ham_sem_wait(ctx->sem)){
		ham_logerrorf(engine_name.ptr, "Error in ham_sem_wait");

		vtable->context_finish(ctx);
		ham_logapiinfof("Finished %.*s context", (int)engine_name.len, engine_name.ptr);

		ham_super(vtable)->on_unload();
		ham_logapiinfof("Unloaded %.*s", (int)engine_name.len, engine_name.ptr);

		ctx->status = 1;
		ham_mutex_unlock(ctx->mut);
		return 1;
	}

	ham_mutex_unlock(ctx->mut);

	if(!ham_sem_wait(ctx->sem)){
		ham_logerrorf(engine_name.ptr, "Error in ham_sem_wait");

		vtable->context_finish(ctx);
		ham_logapiinfof("Finished %.*s context", (int)engine_name.len, engine_name.ptr);

		ham_super(vtable)->on_unload();
		ham_logapiinfof("Unloaded %.*s", (int)engine_name.len, engine_name.ptr);

		ctx->status = 1;
		return 1;
	}

	ham_logapidebugf("Running main loop");

	ham_ticker ticker;
	ham_ticker_reset(&ticker);

	while(ctx->running.load(std::memory_order_relaxed)){
		const ham_f64 dt = ham_ticker_tick(&ticker, ctx->min_dt.load(std::memory_order_relaxed));
		(void)dt;

		vtable->context_loop(ctx, dt);
	}

	vtable->context_finish(ctx);
	ham_logapiinfof("Finished %.*s context", (int)engine_name.len, engine_name.ptr);

	ham_super(vtable)->on_unload();
	ham_logapiinfof("Unloaded %.*s", (int)engine_name.len, engine_name.ptr);

	return 0;
}

ham_engine_context *ham_engine_create(const char *vtable_id, int argc, char **argv){
	ham_path_buffer path_buf;

	const char *game_dir = getcwd(path_buf, HAM_PATH_BUFFER_SIZE);

	while(1){
		int option_index = 0;
		const int getopt_res = getopt_long(argc, argv, "hg:", ham_impl_long_options, &option_index);

		if(getopt_res == -1){
			break;
		}

		switch(getopt_res){
			case 0:{
				if(option_index == 0){
					ham_impl_print_help(argv[0]);
				}
				else if(option_index == 1){
					game_dir = optarg;
				}
				else{
					fprintf(stderr, "Error in getopt_long: unexpected long option at index %d\n", option_index);
					exit(1);
				}

				break;
			}

			case 'h': ham_impl_print_help(argv[0]);

			case 'g':{
				game_dir = optarg;
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
		fprintf(stderr, "Unexpected arguments:");
		while(optind < argc){
			fprintf(stderr, " %s", argv[optind++]);
		}
		fprintf(stderr, "\n");
	}

	const ham::str8 game_path_str = { game_dir, strlen(game_dir) };

	if(!ham_path_exists_utf8(game_path_str)){
		fprintf(stderr, "Game directory does not exist: %s\n", game_dir);
		exit(1);
	}

	constexpr ham::str8 plugin_subdir = "/plugins/";
	constexpr ham::str8 plugin_min_name = "libx" HAM_PLATFORM_DLL_EXT;

	ham_logapiinfof("Game directory: %s", game_dir);

	ham_impl_engine_data data;
	data.vtable_id = ham::str8(vtable_id);

	if((game_path_str.len() + plugin_subdir.len() + plugin_min_name.len()) >= HAM_PATH_BUFFER_SIZE){
		ham_logapierrorf("Game directory too long: %zu, max %zu", game_path_str.len(), HAM_PATH_BUFFER_SIZE - (plugin_subdir.len() + plugin_min_name.len() + 1));
		return ham_null;
	}
	else{
		ham_path_buffer_utf8 game_plugin_path;
		memcpy(game_plugin_path, game_path_str.ptr(), game_path_str.len());
		memcpy(game_plugin_path + game_path_str.len(), plugin_subdir.ptr(), plugin_subdir.len());
		game_plugin_path[game_path_str.len() + plugin_subdir.len()] = '\0';

		if(ham_path_exists_utf8(ham::str8((const char*)game_plugin_path))){
			ham_impl_find_engine_plugin(game_path_str, &data);
		}
	}

	if(!data.vtable){
		const auto self_dll = ham_dll_open_c(nullptr);
		if(!self_dll){
			ham_logapierrorf("Failed to open main executable as shared object");
			return ham_null;
		}

		const auto self_plugin = ham_plugin_load(self_dll);
		if(!self_plugin){
			ham_logapierrorf("Failed to load main executable as plugin");
			ham_dll_close(self_dll);
			return ham_null;
		}

		if(!ham_impl_find_engine_vtable(self_plugin, &data)){
			ham_logapierrorf("Failed to load engine plugin from main executable");
			ham_plugin_unload(self_plugin);
			ham_dll_close(self_dll);
			return ham_null;
		}

		data.dll_handle = self_dll;
	}

	const auto engine_name = ham_super(data.vtable)->name();

	const auto allocator = ham_current_allocator();

	const auto ctx = data.vtable->context_alloc(allocator);
	if(!ctx){
		ham_logapierrorf("Failed to allocate memory for context");
		ham_dll_close(data.dll_handle);
		return ham_null;
	}

	ham_logapidebugf("Allocated %.*s context (%p)", (int)engine_name.len, engine_name.ptr, ctx);

	ctx->allocator  = allocator;
	ctx->dll_handle = data.dll_handle;
	ctx->vtable     = data.vtable;
	ctx->status     = 0;
	ctx->mut        = ham_mutex_create();
	ctx->sem        = ham_sem_create(0);

	if(!ctx->mut || !ctx->sem){
		if(!ctx->mut){
			ham_logapierrorf("Error creating engine mutex");
		}
		else{
			ham_mutex_destroy(ctx->mut);
		}

		if(!ctx->sem){
			ham_logapierrorf("Error creating engine semaphore");
		}
		else{
			ham_sem_destroy(ctx->sem);
		}

		data.vtable->context_free(ctx);
		ham_logapidebugf("Freed %.*s context (%p)", (int)engine_name.len, engine_name.ptr, ctx);

		ham_dll_close(data.dll_handle);
		return ham_null;
	}

	memcpy(ctx->game_dir, game_path_str.ptr(), game_path_str.len());
	ctx->game_dir[game_path_str.len()] = '\0';

	ctx->thd = ham_thread_create(ham_impl_engine_thread_main, ctx);
	if(!ctx->thd){
		ham_logapierrorf("Error creating engine thread");

		ham_mutex_destroy(ctx->mut);
		ham_sem_destroy(ctx->sem);

		data.vtable->context_free(ctx);
		ham_logapidebugf("Freed %.*s context (%p)", (int)engine_name.len, engine_name.ptr, ctx);

		ham_dll_close(data.dll_handle);
		return ham_null;
	}

	ham_sem_wait(ctx->sem);

	return ctx;
}

bool ham_engine_request_exit(ham_engine_context *ctx){
	if(!ham_check(ctx != NULL)) return false;
	ctx->running.store(false, std::memory_order_relaxed);
	return true;
}

int ham_engine_exec(ham_engine_context *ctx){
	if(!ctx) return -1;

	if(!ham_sem_post(ctx->sem)){
		ham_logapierrorf("Error posting engine semaphore");
		ctx->status = -1;
	}
	else if(!ham_mutex_lock(ctx->mut)){
		ham_logapierrorf("Error locking engine mutex");
		ctx->status = -1;
	}

	if(ctx->status == 0){
		ctx->running.store(true, std::memory_order_relaxed);
		if(!ham_sem_post(ctx->sem)){
			ctx->running.store(false, std::memory_order_relaxed);
			ctx->status = 1;
		}
	}

	const auto dll_handle = ctx->dll_handle;
	const auto vtable = ctx->vtable;

	ham_ticker ticker;
	ham_ticker_reset(&ticker);

	while(ctx->running.load(std::memory_order_relaxed)){
		const ham_f64 dt = ham_ticker_tick(&ticker, ctx->min_dt.load(std::memory_order_relaxed));
		(void)dt;

		// TODO
	}

	ham_mutex_unlock(ctx->mut);

	ham_uptr engine_ret;
	if(!ham_thread_join(ctx->thd, &engine_ret)){
		ham_logapierrorf("Error joining engine thread");
		engine_ret = (ham_uptr)-1;
	}
	else if(engine_ret != 0){
		ham_logapierrorf("Error in running engine thread");
		if(ctx->status == 0) ctx->status = 1;
	}

	const int result = ctx->status;

	ham_thread_destroy(ctx->thd);
	ham_mutex_destroy(ctx->mut);
	ham_sem_destroy(ctx->sem);

	vtable->context_free(ctx);

	const auto engine_name = ham_super(vtable)->name();

	ham_logapidebugf("Freed %.*s context (%p)", (int)engine_name.len, engine_name.ptr, ctx);

	ham_super(vtable)->on_unload();
	ham_logapiinfof("Unloaded %.*s", (int)engine_name.len, engine_name.ptr);

	ham_dll_close(dll_handle);
	return result;
}

HAM_C_API_END
