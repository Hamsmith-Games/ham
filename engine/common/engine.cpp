#include "ham/engine.h"

#include "ham/check.h"
#include "ham/fs.h"
#include "ham/async.h"

#include "ham/std_vector.hpp"

using namespace ham::typedefs;

HAM_C_API_BEGIN

struct ham_engine{
	const ham_allocator *allocator;
	ham_engine_app app;
	ham_image *default_tex;

	std::atomic<usize> num_subsystems;
	ham_engine_subsys *subsystems[HAM_ENGINE_MAX_SUBSYSTEMS];

	std::atomic_bool running;
	std::atomic<int> status;
	std::atomic<f64> min_dt;

	ham::std_vector<ham::str_buffer_utf8> interned;
};

ham::mutex ham_impl_gengine_mut;
ham_engine *ham_impl_gengine = nullptr;

ham_public ham_export ham_nothrow ham_u32 ham_net_steam_appid(){
	if(!ham_impl_gengine){
		ham::logapiwarn("Steam networking initialized before engine");
		return 480;
	}

	return ham_impl_gengine->app.id;
}

ham_nothrow ham_version ham_engine_version(){ return HAM_ENGINE_VERSION; }

#define HAM_ENGINE_VERSION_LINE "Ham World Engine - " HAM_ENGINE_BUILD_TYPE_STR " - v" HAM_ENGINE_VERSION_STR "\n"

ham_nothrow const char *ham_engine_version_line(){ return HAM_ENGINE_VERSION_LINE; }

ham_nothrow bool ham_engine_app_load_json(ham_engine_app *ret, const ham_json_value *json_root){
	if(
		!ham_check(ret != NULL) ||
		!ham_check(json_root != NULL) ||
		!ham_check(ham_json_is_object(json_root))
	){
		return false;
	}

	ham::json_value_view json(json_root);

	ham::json_value_view<false>
		app_info, app_id, app_name, app_display, app_author, app_version, app_license, app_desc;

	const auto load_json_value = [](ham::json_value_view<false> *ret, const ham::json_value_view<false> &root, const char *key){
		*ret = root[key];
		if(!*ret){
			ham::logerror("ham_engine_app_load_json", "Invalid application JSON: failed to get value for key '{}'", key);
			return false;
		}
		return true;
	};

	if(!load_json_value(&app_info, json, "app-info")){
		return false;
	}
	else if(!app_info.is_object()){
		ham::logapierror("Invalid application JSON: 'app-info' is not an object");
		return false;
	}

	if(
		!load_json_value(&app_id,      app_info, "id") ||
		!load_json_value(&app_name,    app_info, "name") ||
		!load_json_value(&app_display, app_info, "display-name") ||
		!load_json_value(&app_author,  app_info, "author") ||
		!load_json_value(&app_version, app_info, "version") ||
		!load_json_value(&app_license, app_info, "license") ||
		!load_json_value(&app_desc,    app_info, "description")
	){
		return false;
	}

	if(!app_id.is_nat()){
		ham::logapierror("Invalid application JSON: 'app-info.id' is not a natural number");
		return false;
	}

	if(!app_name.is_str()){
		ham::logapierror("Invalid application JSON: 'app-info.name' is not a string");
		return false;
	}

	if(!app_display.is_str()){
		ham::logapierror("Invalid application JSON: 'app-info.display-name' is not a string");
		return false;
	}

	if(!app_author.is_str()){
		ham::logapierror("Invalid application JSON: 'app-info.author' is not a string");
	}

	if(!app_version.is_array() || app_version.array_len() != 3){
		ham::logapierror("Invalid application JSON: 'app-info.version' is not a 3 element array");
		return false;
	}

	const usize iterate_res = app_version.array_iterate([](usize idx, const ham::json_value_view<false> &val) -> bool{
		if(!val.is_nat()){
			ham::logerror("ham_engine_app_load_json", "Invalid application JSON: 'app-info.version[{}]' is not a natural number", idx);
			return false;
		}

		return true;
	});

	if(iterate_res != 3){
		return false;
	}

	if(!app_license.is_str()){
		ham::logapierror("Invalid application JSON: 'app-info.license' is not a string");
		return false;
	}

	if(!app_desc.is_str()){
		ham::logapierror("Invalid application JSON: 'app-info.description' is not a string");
		return false;
	}

	ret->id = app_id.get_nat();
	ret->name = app_name.get_str();
	ret->display_name = app_display.get_str();
	ret->author = app_author.get_str();
	ret->license = app_license.get_str();
	ret->description = app_desc.get_str();

	ret->version.major = app_version[0].get_nat();
	ret->version.minor = app_version[1].get_nat();
	ret->version.patch = app_version[2].get_nat();

	return true;
}

static inline void ham_image_buf_fill_rgba8(
	ham_u8 *buf, ham_u32 buf_w, ham_u32 buf_h,
	ham_color_u8 color,
	ham_u32 off_x, ham_u32 off_y,
	ham_u32 fill_w, ham_u32 fill_h
){

#ifdef __AVX__
	alignas(32) const ham_u32 color8[8] = {
		color.bits, color.bits, color.bits, color.bits,
		color.bits, color.bits, color.bits, color.bits,
	};
	const __m256i pix8 = _mm256_load_si256((const __m256i*)color8);
	const ham_u32 fill_w_step = 8;
#elif defined(__x86_64__)
	alignas(16) const ham_u32 color4[4] = { color.bits, color.bits, color.bits, color.bits };
	const __m128i pix4 = _mm_load_si128((const __m128i*)color4);
	const ham_u32 fill_w_step = 4;
#else
	ham_v4u32 pix4 = { color.bits, color.bits, color.bits, color.bits };
	const ham_u32 fill_w_step = 4;
#endif

	const ham_u32 fill_w_div = fill_w / fill_w_step;
	const ham_u32 rem_w = fill_w % fill_w_step;

	for(ham_u32 y = 0; y < fill_h; y++){
		const auto y_idx = (off_y + y) * buf_w;

		for(ham_u32 x = 0; x < fill_w_div; x++){
			const auto x_idx = off_x + (x * fill_w_step); // 'fill_w_step' color stride

			const auto buf_idx = (y_idx + x_idx) * 4; // 4 component colors

		#ifdef __AVX__
			_mm256_storeu_si256((__m256i*)(buf + buf_idx), pix8);
		#elif defined(__x86_64__)
			_mm_storeu_si128((__m128i*)(buf + buf_idx), pix4);
		#else
			memcpy(buf + buf_idx, &pix4, sizeof(pix4));
		#endif
		}

		for(ham_u32 x = 0; x < rem_w; x++){
			const auto x_idx = off_x + (fill_w_div * fill_w_step) + x;
			const auto buf_idx = (y_idx + x_idx) * 4;

			memcpy(buf + buf_idx, &color, sizeof(color));
		}
	}
}

ham_engine *ham_engine_create(const ham_engine_app *app){
	ham::scoped_lock lock(ham_impl_gengine_mut);

	if(
		!ham_check(app != NULL) ||
		!ham_check(app->init != NULL) ||
		!ham_check(app->fini != NULL) ||
		!ham_check(app->loop != NULL) ||
		!ham_check(ham_gengine() == NULL)
	){
		return nullptr;
	}

	const ham::str8 app_dir = app->dir;

	if(!ham::path_exists(app_dir)){
		ham::logapierror("App directory does not exist: {}", app_dir);
		return nullptr;
	}

	ham_u8 default_tex_pixels[512*512*4];
	constexpr ham_color_u8 pixel_color = { .data = { 0xff, 0x78, 0x0, 0xff } };
	constexpr ham_color_u8 pixel_color_grey = { .data = { 0x69, 0x69, 0x69, 0xff } };

	ham_image_buf_fill_rgba8(default_tex_pixels, 512, 512, pixel_color, 0, 0, 256, 256);
	ham_image_buf_fill_rgba8(default_tex_pixels, 512, 512, pixel_color, 256, 256, 256, 256);

	ham_image_buf_fill_rgba8(default_tex_pixels, 512, 512, pixel_color_grey, 256, 0, 256, 256);
	ham_image_buf_fill_rgba8(default_tex_pixels, 512, 512, pixel_color_grey, 0, 256, 256, 256);

	const auto default_tex = ham_image_create(HAM_RGBA8U, 512, 512, default_tex_pixels);
	if(!default_tex){
		ham::logapierror("Could not create default texture");
		return nullptr;
	}

	const auto allocator = ham_current_allocator();

	const auto engine = ham_allocator_new(allocator, ham_engine);
	if(!engine){
		ham_image_destroy(default_tex);
		return nullptr;
	}

	const auto dir_str = engine->interned.emplace_back(app->dir).c_str();
	const auto name_str = engine->interned.emplace_back(app->name).c_str();
	const auto display_str = engine->interned.emplace_back(app->display_name).c_str();
	const auto author_str = engine->interned.emplace_back(app->author).c_str();
	const auto license_str = engine->interned.emplace_back(app->license).c_str();
	const auto desc_str = engine->interned.emplace_back(app->description).c_str();

	engine->app.id = app->id;
	engine->app.dir = ham::str8(dir_str);
	engine->app.name = ham::str8(name_str);
	engine->app.display_name = ham::str8(display_str);
	engine->app.author = ham::str8(author_str);
	engine->app.license = ham::str8(license_str);
	engine->app.description = ham::str8(desc_str);
	engine->app.version = app->version;
	engine->app.init = app->init;
	engine->app.fini = app->fini;
	engine->app.loop = app->loop;
	engine->app.user = app->user;

	engine->allocator = allocator;
	engine->default_tex = default_tex;

	engine->num_subsystems.store(0, std::memory_order_relaxed);
	memset(engine->subsystems, 0, sizeof(engine->subsystems));

	engine->running.store(false, std::memory_order_relaxed);

	engine->min_dt.store(1.0/60.0, std::memory_order_relaxed);

	if(!app->init(engine, app->user)){
		ham::logapierror("Failed to initialize app '{}'", app->name);
		ham_allocator_delete(allocator, engine);
		return nullptr;
	}

	ham_impl_gengine = engine;

	return engine;
}

ham_nothrow void ham_engine_destroy(ham_engine *engine){
	if(ham_unlikely(!engine)) return;

	ham::scoped_lock lock(ham_impl_gengine_mut);

	int exit_status = 0;

	if(engine->running.exchange(false)){
		ham::logapiwarn("Running engine destroyed");
		engine->status.compare_exchange_strong(exit_status, 3);
	}

	if(exit_status != 0){
		ham::logapiwarn("Engine exited with error code {}", exit_status);
	}

	const auto allocator = engine->allocator;

	const auto app = &engine->app;

	app->fini(engine, app->user);

	const auto num_subsys = engine->num_subsystems.load(std::memory_order_relaxed);

	for(ham_usize i = 0; i < num_subsys; i++){
		const auto subsys = engine->subsystems[i];
		ham_impl_engine_subsys_destroy(subsys);
	}

	ham_image_destroy(engine->default_tex);

	ham_allocator_delete(allocator, engine);

	ham_impl_gengine = nullptr;
}

ham_nothrow const ham_engine_app *ham_engine_get_app(const ham_engine *engine){
	if(!ham_check(engine != NULL)) return nullptr;

	return &engine->app;
}

ham_nothrow ham_usize ham_engine_num_subsystems(const ham_engine *engine){
	if(!ham_check(engine != NULL)) return (ham_usize)-1;

	return engine->num_subsystems;
}

ham_nothrow ham_engine_subsys *ham_engine_get_subsystem(ham_engine *engine, ham_usize idx){
	if(!ham_check(engine != NULL) || !ham_check(idx < engine->num_subsystems)){
		return nullptr;
	}

	return engine->subsystems[idx];
}

ham_nothrow const ham_image *ham_engine_get_default_tex_image(const ham_engine *engine){
	if(!ham_check(engine != NULL)) return nullptr;
	return engine->default_tex;
}

ham_nothrow bool ham_engine_request_exit(ham_engine *engine){
	if(!ham_check(engine != NULL)) return false;

	engine->running.store(false, std::memory_order_relaxed);
	return true;
}

int ham_engine_exec(ham_engine *engine){
	engine->running.store(true, std::memory_order::relaxed);
	engine->status.store(0, std::memory_order_relaxed);

	for(ham_usize i = 0; i < engine->num_subsystems; i++){
		const auto subsys = engine->subsystems[i];
		if(!ham_engine_subsys_running(subsys) && !ham_engine_subsys_launch(subsys)){
			ham::logapierror("Error launching subsystem '{}'", ham_engine_subsys_name(subsys));
			engine->running.store(false, std::memory_order::relaxed);
			engine->status.store(2, std::memory_order_relaxed);
			break;
		}
	}

	const auto app = &engine->app;

	ham_ticker ticker;
	ham_ticker_reset(&ticker);

	f64 dt = 0.0;

	while(engine->running.load(std::memory_order::relaxed)){
		const f64 min_dt = engine->min_dt.load(std::memory_order_relaxed);

		dt = 0.0;
		do{
			dt += ham_ticker_tick(&ticker, min_dt - dt);
		} while(dt < min_dt);

		app->loop(engine, dt, app->user);
	}

	return engine->status.load();
}

//
// Subsystems
//

struct ham_engine_subsys{
	ham_engine *engine;
	ham::str_buffer_utf8 name;

	ham_engine_subsys_init_fn init_fn;
	ham_engine_subsys_fini_fn fini_fn;
	ham_engine_subsys_loop_fn loop_fn;
	void *user;

	std::atomic_bool running;
	std::atomic<f64> min_dt;

	ham::thread thread;
	ham::mutex mut;
	ham::cond cond;
};

uptr ham_impl_engine_subsys_routine(void *user){
	const auto subsys = reinterpret_cast<ham_engine_subsys*>(user);

	{
		ham::unique_lock lock(subsys->mut);
		if(!subsys->cond.wait(lock, [subsys]{ return subsys->running.load(std::memory_order_relaxed); })){
			ham::logapierror("Error waiting on subsystem mutex");
			return -1;
		}
	}

	if(!subsys->init_fn(subsys->engine, subsys->user)){
		ham::logapierror("Error initializing subsystem '{}'", subsys->name);
		return 1;
	}

	ham_ticker ticker;
	ham_ticker_reset(&ticker);

	ham_f64 dt;

	while(subsys->running.load(std::memory_order_relaxed)){
		const f64 target_dt = subsys->min_dt.load(std::memory_order_relaxed);

		dt = 0.0;
		do{
			dt += ham_ticker_tick(&ticker, target_dt - dt);
		} while(dt < target_dt);

		subsys->loop_fn(subsys->engine, dt, subsys->user);
	}

	subsys->fini_fn(subsys->engine, subsys->user);

	return 0;
}

ham_engine_subsys *ham_engine_subsys_create(
	ham_engine *engine,
	ham_str8 name,
	ham_engine_subsys_init_fn init_fn,
	ham_engine_subsys_fini_fn fini_fn,
	ham_engine_subsys_loop_fn loop_fn,
	void *user
){
	if(
		!ham_check(engine != NULL) ||
		!ham_check(name.ptr && name.len) ||
		!ham_check(init_fn != NULL) ||
		!ham_check(fini_fn != NULL) ||
		!ham_check(loop_fn != NULL)
	){
		return nullptr;
	}

	const auto allocator = engine->allocator;

	const auto subsys = ham_allocator_new(allocator, ham_engine_subsys);
	if(!subsys){
		return nullptr;
	}

	const auto subsys_idx = engine->num_subsystems.fetch_add(1, std::memory_order_relaxed);
	if(subsys_idx >= HAM_ENGINE_MAX_SUBSYSTEMS){
		ham::logapierror("Maximum number of subsystems created ({})", HAM_ENGINE_MAX_SUBSYSTEMS);
		ham_allocator_delete(allocator, subsys);
		return nullptr;
	}

	engine->subsystems[subsys_idx] = subsys;

	subsys->engine = engine;
	subsys->name   = name;

	subsys->init_fn = init_fn;
	subsys->fini_fn = fini_fn;
	subsys->loop_fn = loop_fn;
	subsys->user    = user;

	subsys->running.store(false, std::memory_order_relaxed);
	subsys->min_dt.store(engine->min_dt);

	subsys->thread = ham::thread(ham_impl_engine_subsys_routine, subsys);

	return subsys;
}

ham_nothrow void ham_impl_engine_subsys_destroy(ham_engine_subsys *subsys){
	if(ham_unlikely(!subsys)) return;

	if(subsys->running.exchange(false)){
		ham::logapiwarn("Running subsystem destroyed '{}'", subsys->name);
	}

	uptr ret;
	if(!subsys->thread.join(&ret)){
		ham::logapiwarn("Failed to join thread for subsystem '{}'", subsys->name);
		ret = (uptr)-1;
	}

	if(ret != 0){
		ham::logapiwarn("Subsystem '{}' returned with exit code {}", subsys->name, ret);
	}

	const auto allocator = subsys->engine->allocator;

	ham_allocator_delete(allocator, subsys);
}

ham_nothrow ham_engine *ham_engine_subsys_owner(ham_engine_subsys *subsys){
	if(!ham_check(subsys != NULL)) return nullptr;
	return subsys->engine;
}

ham_nothrow ham_str8 ham_engine_subsys_name(const ham_engine_subsys *subsys){
	if(!ham_check(subsys != NULL)) return HAM_EMPTY_STR8;
	return subsys->name.get();
}

ham_nothrow bool ham_engine_subsys_running(const ham_engine_subsys *subsys){
	if(!ham_check(subsys != NULL)) return false;
	return subsys->running.load(std::memory_order_relaxed);
}

ham_nothrow bool ham_engine_subsys_set_min_dt(ham_engine_subsys *subsys, ham_f64 min_dt){
	if(!ham_check(subsys != NULL)) return false;
	subsys->min_dt.store(ham_max(0.0, min_dt), std::memory_order_relaxed);
	return true;
}

ham_nothrow bool ham_engine_subsys_launch(ham_engine_subsys *subsys){
	if(!ham_check(subsys != NULL)) return false;

	{
		ham::scoped_lock lock(subsys->mut);
		if(subsys->running.exchange(true)) return true;
	}

	if(!subsys->cond.signal()){
		ham::logapierror("Error signaling subsystem thread");
		subsys->running.store(false);
		return false;
	}

	return true;
}

HAM_C_API_END
