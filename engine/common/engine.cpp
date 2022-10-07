/*
 * Ham World Engine Runtime
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
#include "ham/engine-object.h"
#include "ham/memory.h"
#include "ham/plugin.h"
#include "ham/check.h"
#include "ham/fs.h"
#include "ham/json.h"

#include <unistd.h>
#include <getopt.h>
#include <dirent.h>

HAM_C_API_BEGIN

static ham_engine *ham_impl_gengine = nullptr;

ham_extern_c ham_public ham_export ham_nothrow ham_u32 ham_net_steam_appid(){
	if(!ham_impl_gengine){
		ham_logapiwarnf("Steam networking initialized before engine");
		return 480;
	}

	return ham_impl_gengine->app_info.appid;
}

struct ham_impl_engine_data{
	ham::str8 vtable_id;
	ham_dso_handle dso_handle = nullptr;
	ham_plugin *plugin = nullptr;
	const ham_engine_vtable *vtable = nullptr;
	ham_app_info app_info;
};

static inline ham_uptr ham_impl_engine_thread_main(void *user){
	const auto engine = reinterpret_cast<ham_engine*>(user);
	const auto vtable = ham_super(engine)->vtable;

	const char *engine_name = vtable->info()->type_id;

	ham_loginfof(
		"ham-engine",
		"Creating engine for app: %s v%u.%u.%u",
		engine->app_info.display_name,
		engine->app_info.version.major,
		engine->app_info.version.minor,
		engine->app_info.version.patch
	);

	ham_logdebugf("ham-engine", "Waiting for engine mutex...");

	if(!ham_mutex_lock(engine->mut)){
		ham_logerrorf("ham-engine", "Error locking engine mutex");
		engine->status = 1;
		return 1;
	}

	if(!ham_plugin_init(engine->plugin)){
		ham_logerrorf("ham-engine", "Error initializing plugin '%s'", engine_name);
		engine->status = 1;
		ham_mutex_unlock(engine->mut);
		return 1;
	}

	ham_logdebugf("ham-engine", "Initialized plugin");

	const auto engine_vt = (const ham_engine_vtable*)vtable;

	if(!vtable->ctor(ham_super(engine), 0, nullptr)){
		ham_logerrorf("ham-engine", "Error constructing engine object '%s' (%p)", engine_name, engine);

		ham_plugin_fini(engine->plugin);
		ham_logdebugf("ham-engine", "Finished plugin (%p)", engine->plugin);

		engine->status = 1;
		ham_mutex_unlock(engine->mut);
		return 1;
	}

	ham_logdebugf("ham-engine", "Constructed engine object '%s' (%p)", engine_name, engine);

	if(!engine_vt->init(engine)){
		ham_logerrorf("ham-engine", "Error initializing engine object '%s' (%p)", engine_name, engine);

		vtable->dtor(ham_super(engine));
		ham_logdebugf("ham-engine", "Destroyed engine object '%s' (%p)", engine_name, engine);

		ham_plugin_fini(engine->plugin);
		ham_logdebugf("ham-engine", "Finished plugin (%p)", engine->plugin);

		engine->status = 1;
		ham_mutex_unlock(engine->mut);
		return 1;
	}

	ham_logdebugf("ham-engine", "Initialized engine object '%s' (%p)", engine_name, engine);

	ham_logdebugf("ham-engine", "Posting engine semaphore...");

	if(!ham_sem_post(engine->sem)){
		ham_logerrorf("ham-engine", "Error posting engine semaphore");
		engine->status = 1;

		engine_vt->fini(engine);
		ham_logdebugf("ham-engine", "Finished engine object '%s' (%p)", engine_name, engine);

		vtable->dtor(ham_super(engine));
		ham_logdebugf("ham-engine", "Destroyed engine object '%s' (%p)", engine_name, engine);

		ham_plugin_fini(engine->plugin);
		ham_logdebugf("ham-engine", "Finished plugin (%p)", engine->plugin);

		return 1;
	}

	ham_logdebugf("ham-engine", "Waiting for engine launch condition...");

	// prevent spurious wakeups
	while(!engine->running){
		if(!ham_cond_wait(engine->cond, engine->mut)){
			ham_logerrorf("ham-engine", "Error in ham_cond_wait");

			engine_vt->fini(engine);
			ham_logdebugf("ham-engine", "Finished engine object '%s' (%p)", engine_name, engine);

			vtable->dtor(ham_super(engine));
			ham_logdebugf("ham-engine", "Destroyed engine object '%s' (%p)", engine_name, engine);

			ham_plugin_fini(engine->plugin);
			ham_logdebugf("ham-engine", "Finished plugin (%p)", engine->plugin);

			engine->status = 1;
			return 1;
		}
	}

	ham_mutex_unlock(engine->mut);
	ham_logdebugf("ham-engine", "Unlocked engine mutex");

	ham_logdebugf("ham-engine", "Running main loop");

	ham_ticker ticker;
	ham_ticker_reset(&ticker);

	ham_u32 total_frame_count = 0, frame_count = 0;
	ham_f64 frame_dt_accum = 0.0;

	const auto loop_fn = engine_vt->loop;

	while(engine->running){
		const auto req_min_dt = engine->min_dt;

		ham_f64 dt = ham_ticker_tick(&ticker, req_min_dt);
		while(dt < req_min_dt){
			dt += ham_ticker_tick(&ticker, req_min_dt - dt);
		}

		loop_fn(engine, dt);

		++frame_count;
		frame_dt_accum += dt;

		if(frame_dt_accum >= 1.0){
			const auto fps = (ham_f64)frame_count / frame_dt_accum;

			ham_logdebugf("ham-engine", "FPS: %f", fps);

			total_frame_count += frame_count;
			frame_count = 0;
			frame_dt_accum = 0.0;
		}
	}

	ham_sem_wait(engine->sem);

	engine_vt->fini(engine);
	ham_logdebugf("ham-engine", "Finished engine object '%s' (%p)", engine_name, engine);

	vtable->dtor(ham_super(engine));
	ham_logdebugf("ham-engine", "Destroyed engine object '%s' (%p)", engine_name, engine);

	ham_plugin_fini(engine->plugin);
	ham_logdebugf("ham-engine", "Finished plugin (%p)", engine->plugin);

	return 0;
}

ham_nothrow ham_version ham_engine_version(){ return HAM_ENGINE_VERSION; }

#define HAM_ENGINE_VERSION_LINE "Ham World Engine - " HAM_ENGINE_BUILD_TYPE_STR " - v" HAM_ENGINE_VERSION_STR "\n"

ham_nothrow const char *ham_engine_version_line(){ return HAM_ENGINE_VERSION_LINE; }

[[noreturn]]
static inline void ham_impl_show_version(){
	printf(HAM_ENGINE_VERSION_LINE);
	exit(EXIT_SUCCESS);
}

[[noreturn]]
static void ham_impl_show_help(const char *argv0){
	const char *exec_name = argv0;

	const auto argv0_str = ham::str8(argv0);
	const auto new_beg = argv0_str.rfind("/");
	if(new_beg != ham::str8::npos){
		exec_name += new_beg;
	}

	printf(
		HAM_ENGINE_VERSION_LINE
		"\n"
		"    Usage: %s [OPTIONS...] [-- [SERVER-ARGS...]]\n"
		"\n"
		"    Possible options:\n"
		"        -h|--help:             Print this help message and exit\n"
		"        -v|--version:          Print the version string and exit\n"
		"        -V|--verbose:          Print more log messages\n"
		"        -a|--app-dir DIR:      Set the game directory\n"
		"\n"
		,
		exec_name
	);
	exit(EXIT_SUCCESS);
}

static struct option ham_impl_long_options[] = {
	{ "help",		no_argument,		0, 0 },
	{ "version",	no_argument,		0, 0 },
	{ "verbose",	no_argument,		0, 0 },
	{ "app-dir",	required_argument,	0, 0 },
	{ 0,            0,                  0, 0 },
};

// TODO: not get app info from a JSON file
static inline bool ham_app_info_init(ham::str8 json_path, ham_app_info *ret){
	const auto json_doc = ham::json_document::open(json_path);
	if(!json_doc){
		ham_logapierrorf("Failed to open app JSON: %.*s", (int)json_path.len(), json_path.ptr());
		return false;
	}

	const auto json_root = json_doc.root();

	const auto appinfo = json_root.object_get("app-info");
	if(!appinfo){
		ham_logapierrorf("Failed to get \"app-info\" object from app JSON");
		return false;
	}

	const auto appinfo_get = [appinfo](ham::json_value_view<false> *ret, const char *elem) -> bool{
		*ret = appinfo.object_get(elem);
		if(!*ret){
			ham_logapierrorf("Failed to get \"app-info.%s\" from app JSON", elem);
			return false;
		}

		return true;
	};

	ham::json_value_view<false>
		appinfo_id,
		appinfo_name,
		appinfo_display_name,
		appinfo_author,
		appinfo_version,
		appinfo_license,
		appinfo_desc
	;
	if(
		!appinfo_get(&appinfo_id,           "id") ||
		!appinfo_get(&appinfo_name,         "name") ||
		!appinfo_get(&appinfo_display_name, "display-name") ||
		!appinfo_get(&appinfo_author,       "author") ||
		!appinfo_get(&appinfo_version,       "version") ||
		!appinfo_get(&appinfo_license,      "license") ||
		!appinfo_get(&appinfo_desc,         "desc")
	){
		return false;
	}

	// Value checking

	if(!appinfo_id.is_nat()){
		ham_logapierrorf("Invalid type for \"app-info.id\": expected uint");
		return false;
	}

	const auto version_len = appinfo_version.array_len();
	if(version_len != 3){
		ham_logapierrorf("Invalid version array: bad length (expected 3)");
		return false;
	}

	const auto appinfo_ver_major = appinfo_version.array_get(0);
	const auto appinfo_ver_minor = appinfo_version.array_get(1);
	const auto appinfo_ver_patch = appinfo_version.array_get(2);

	if(
	   !appinfo_ver_major.is_nat() ||
	   !appinfo_ver_minor.is_nat() ||
	   !appinfo_ver_patch.is_nat()
	){
		ham_logapierrorf("Invalid version array: bad element type (expected uint)");
		return false;
	}

	// TODO: check if strings overflow or mayber use str_buffer_utf8

	// Write good values

	ret->appid = appinfo_id.get_nat();

	ret->version.major = appinfo_ver_major.get_nat();
	ret->version.minor = appinfo_ver_minor.get_nat();
	ret->version.patch = appinfo_ver_patch.get_nat();

	const auto name = appinfo_name.get_str();
	memcpy(ret->name, name.ptr(), name.len());
	ret->name[name.len()] = '\0';

	const auto display_name = appinfo_display_name.get_str();
	memcpy(ret->display_name, display_name.ptr(), display_name.len());
	ret->display_name[display_name.len()] = '\0';

	const auto author = appinfo_author.get_str();
	memcpy(ret->author, author.ptr(), author.len());
	ret->author[author.len()] = '\0';

	return true;
}

ham_engine *ham_engine_create(const char *vtable_id, const char *obj_id, int argc, char **argv){
	ham_path_buffer path_buf;

	const char *app_dir = getcwd(path_buf, HAM_PATH_BUFFER_SIZE);

	while(1){
		int option_index = 0;
		const int getopt_res = getopt_long(argc, argv, "hvVa:", ham_impl_long_options, &option_index);

		if(getopt_res == -1){
			break;
		}

		switch(getopt_res){
			case 0:{
				switch(option_index){
					case 0: ham_impl_show_help(argv[0]);
					case 1: ham_impl_show_version();

					case 2:{
						ham_log_set_verbose(true);
						break;
					}

					case 3:{
						app_dir = optarg;
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

			case 'V':{
				ham_log_set_verbose(true);
				break;
			}

			case 'a':{
				app_dir = optarg;
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

	const ham::str8 app_path_str = { app_dir, strlen(app_dir) };

	if(!ham_path_exists_utf8(app_path_str)){
		fprintf(stderr, "App directory does not exist: %s\n", app_dir);
		exit(1);
	}

	constexpr ham::str8 plugin_subdir = "/plugins/";
	constexpr ham::str8 plugin_min_name = "libx" HAM_PLATFORM_DSO_EXT;

	ham_logapiinfof("App directory: %s", app_dir);

	constexpr ham::str8 app_json_name = "ham.json";
	const ham_usize app_json_path_len = (app_path_str.len() + app_json_name.len() + 1);

	if(app_json_path_len >= HAM_PATH_BUFFER_SIZE){
		ham_logapierrorf("App JSON path too long (%zu, max %d): %s/%s", app_json_path_len, HAM_PATH_BUFFER_SIZE, app_dir, app_json_name.ptr());
		return nullptr;
	}

	ham_path_buffer_utf8 app_json_path;
	memcpy(app_json_path, app_path_str.ptr(), app_path_str.len());
	app_json_path[app_path_str.len()] = '/';
	memcpy(app_json_path + app_path_str.len() + 1, app_json_name.ptr(), app_json_name.len());
	app_json_path[app_json_path_len] = '\0';

	if(!ham_path_exists_utf8(ham::str8(app_json_path, app_json_path_len))){
		ham_logapierrorf("App JSON does not exist: %s", app_json_path);
		return nullptr;
	}

	ham_impl_engine_data data{};

	if(!ham_app_info_init({ app_json_path, app_json_path_len }, &data.app_info)){
		ham_logapierrorf("Failed to initialize app info from JSON");
		return nullptr;
	}

	data.vtable_id = ham::str8(vtable_id);

	if((app_path_str.len() + plugin_subdir.len() + plugin_min_name.len()) >= HAM_PATH_BUFFER_SIZE){
		ham_logapierrorf("App directory too long: %zu, max %zu", app_path_str.len(), HAM_PATH_BUFFER_SIZE - (plugin_subdir.len() + plugin_min_name.len() + 1));
		return nullptr;
	}
	else{
		ham_path_buffer_utf8 app_plugin_path;
		memcpy(app_plugin_path, app_path_str.ptr(), app_path_str.len());
		memcpy(app_plugin_path + app_path_str.len(), plugin_subdir.ptr(), plugin_subdir.len());
		app_plugin_path[app_path_str.len() + plugin_subdir.len()] = '\0';

		if(ham_path_exists_utf8(ham::str8((const char*)app_plugin_path))){
			if(ham_plugin_find(vtable_id, ham::str8((const char*)app_plugin_path), &data.plugin, &data.dso_handle)){
				data.vtable = (const ham_engine_vtable*)ham_plugin_object(data.plugin, ham::str8(obj_id));
				if(!data.vtable){
					ham_plugin_unload(data.plugin);
					ham_dso_close(data.dso_handle);
					data.plugin = nullptr;
					data.dso_handle = nullptr;
				}
			}
		}
	}

	if(!data.vtable){
		const auto self_dso = ham_dso_open_c(nullptr, 0);
		if(!self_dso){
			ham_logapierrorf("Failed to open main executable as shared object");
			return nullptr;
		}

		const auto self_plugin = ham_plugin_load(self_dso, vtable_id);
		if(!self_plugin){
			ham_logapierrorf("Failed to load main executable as requested plugin: %s", vtable_id);
			ham_dso_close(self_dso);
			return nullptr;
		}

		const auto obj_vtable = (const ham_engine_vtable*)ham_plugin_object(self_plugin, ham::str8(obj_id));
		if(!obj_vtable){
			ham_logapierrorf("Failed to find engine object by id: %s", obj_id);
			ham_plugin_unload(self_plugin);
			ham_dso_close(self_dso);
			return nullptr;
		}

		data.dso_handle = self_dso;
		data.plugin = self_plugin;
		data.vtable = obj_vtable;
	}

	const auto plugin_display_name = ham_plugin_display_name(data.plugin);
	const auto plugin_version      = ham_plugin_version(data.plugin);

	ham_logapiinfof("Engine plugin: %s v%d.%d.%d", plugin_display_name.ptr, plugin_version.major, plugin_version.minor, plugin_version.patch);

	const auto engine_info = ham_super(data.vtable)->info();
	//const auto engine_name = engine_info->type_id;

	const auto allocator = ham_current_allocator();

	const auto obj = (ham_object*)ham_allocator_alloc(allocator, engine_info->alignment, engine_info->size);
	if(!obj){
		ham_logapierrorf("Failed to allocate memory for engine");
		ham_plugin_unload(data.plugin);
		ham_dso_close(data.dso_handle);
		return nullptr;
	}

	memset(obj, 0, engine_info->size);

	obj->vtable = ham_super(data.vtable);

	const auto engine = (ham_engine*)obj;

	engine->allocator  = allocator;
	engine->plugin     = data.plugin;
	engine->dso_handle = data.dso_handle;
	engine->status     = 0;
	engine->min_dt     = 1.0/60.0;
	engine->mut        = ham_mutex_create(HAM_MUTEX_NORMAL);
	engine->sem        = ham_sem_create(0);
	engine->cond       = ham_cond_create();
	engine->subsys_mut = ham_mutex_create(HAM_MUTEX_NORMAL);

	if(!engine->mut || !engine->sem || !engine->cond || !engine->subsys_mut){
		if(!engine->mut){
			ham_logapierrorf("Error creating engine mutex");
		}
		else{
			ham_mutex_destroy(engine->mut);
		}

		if(!engine->sem){
			ham_logapierrorf("Error creating engine semaphore");
		}
		else{
			ham_sem_destroy(engine->sem);
		}

		if(!engine->cond){
			ham_logapierrorf("Error creating engine condition variable");
		}
		else{
			ham_cond_destroy(engine->cond);
		}

		if(!engine->subsys_mut){
			ham_logapierrorf("Error creating engine subsystem mutex");
		}
		else{
			ham_mutex_destroy(engine->subsys_mut);
		}

		ham_allocator_free(allocator, obj);

		ham_plugin_unload(data.plugin);
		ham_dso_close(data.dso_handle);
		return nullptr;
	}

	memcpy(&engine->app_info, &data.app_info, sizeof(ham_app_info));

	memcpy(engine->game_dir, app_path_str.ptr(), app_path_str.len());
	engine->game_dir[app_path_str.len()] = '\0';

	engine->thd = ham_thread_create(ham_impl_engine_thread_main, engine);
	if(!engine->thd){
		ham_logapierrorf("Error creating engine thread");

		ham_mutex_destroy(engine->mut);
		ham_sem_destroy(engine->sem);
		ham_cond_destroy(engine->cond);
		ham_mutex_destroy(engine->subsys_mut);

		ham_allocator_free(allocator, obj);

		ham_plugin_unload(data.plugin);
		ham_dso_close(data.dso_handle);
		return nullptr;
	}

	ham_sem_wait(engine->sem);

	return engine;
}

ham_nothrow void ham_engine_destroy(ham_engine *engine){
	if(ham_unlikely(engine == NULL)) return;

	for(int i = 0; i < engine->num_subsystems; i++){
		const auto subsys = engine->subsystems[i];
		ham_impl_engine_subsys_destroy(subsys);
	}

	ham_thread_destroy(engine->thd);
	ham_mutex_destroy(engine->mut);
	ham_sem_destroy(engine->sem);
	ham_cond_destroy(engine->cond);
	ham_mutex_destroy(engine->subsys_mut);

	ham_plugin_unload(engine->plugin);
	ham_dso_close(engine->dso_handle);

	ham_allocator_free(engine->allocator, engine);
}

ham_nothrow bool ham_engine_request_exit(ham_engine *engine){
	if(!ham_check(engine != NULL)) return false;
	engine->running = false;

	return true;
}

int ham_engine_exec(ham_engine *engine){
	if(!engine) return -1;

	ham_impl_gengine = engine;

	if(engine->status != 0){
		ham_engine_destroy(engine);
		return engine->status;
	}

	if(!ham_mutex_lock(engine->mut)){
		ham_logapierrorf("Error locking engine mutex");
		ham_engine_destroy(engine);
		return 1;
	}
	else if(!ham_cond_broadcast(engine->cond)){
		ham_logapierrorf("Error signaling engine thread");
		ham_mutex_unlock(engine->mut);
		ham_engine_destroy(engine);
		return 1;
	}
	else{
		ham::scoped_lock lock(engine->subsys_mut);
		for(int i = 0; i < engine->num_subsystems; i++){
			const auto subsys = engine->subsystems[i];
			if(!ham_engine_subsys_launch(subsys)){
				ham_logapierrorf("Failed to launch subsystem %d", i);
				ham_mutex_unlock(engine->mut);
				ham_engine_destroy(engine);
				return 2;
			}
		}
	}

	engine->running = true;
	ham_mutex_unlock(engine->mut);

	ham_ticker ticker;
	ham_ticker_reset(&ticker);

	while(engine->running){
		const ham_f64 dt = ham_ticker_tick(&ticker, engine->min_dt);
		(void)dt;

		// TODO
	}

	for(int i = 0; i < engine->num_subsystems; i++){
		const auto subsys = engine->subsystems[i];
		ham_impl_engine_subsys_request_exit(subsys);
	}

	ham_sem_post(engine->sem);

	ham_uptr engine_ret;
	if(!ham_thread_join(engine->thd, &engine_ret)){
		ham_logapierrorf("Error joining engine thread");
		engine_ret = (ham_uptr)-1;
	}
	else if(engine_ret != 0){
		ham_logapierrorf("Error in running engine thread");
		if(engine->status == 0) engine->status = 1;
	}

	const int result = engine->status;

	ham_impl_gengine = nullptr;

	ham_engine_destroy(engine);

	return result;
}

HAM_C_API_END
