#include "client.h"

#include "ham/log.h"

using namespace ham::typedefs;

namespace engine = ham::engine;

bool engine::client_video_subsystem::init(ham_engine *engine){
	// TODO: initialize vulkan or GL depending on engine config

	auto new_win = ham::make_owned<engine::client_window_gl>(ham_engine_get_app(engine));
	if(!new_win){
		ham::logapierror("Error allocating new engine::client_window_gl");
		return false;
	}

	m_win = std::move(new_win);

	ham::logapiverbose("Video subsystem initialized");
	return true;
}

void engine::client_video_subsystem::fini(ham_engine *engine){
	(void)engine;

	ham_renderer_destroy(m_r);

	m_win.destroy();

	ham::logapiverbose("Video subsystem finished");
}

void engine::client_video_subsystem::loop(ham_engine *engine, f64 dt){
	(void)engine;
	m_win->present(dt, m_r);
}
