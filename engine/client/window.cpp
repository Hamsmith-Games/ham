#include "client.h"

#include "ham/log.h"
#include "ham/buffer.h"

using namespace ham::typedefs;

namespace engine = ham::engine;

//
// Client window base class
//

engine::client_window::client_window(const ham_engine_app *app, Uint32 flags){
	auto title_str = ham::format("Ham Engine Client - {}", app->display_name);

	m_win = SDL_CreateWindow(
		title_str.c_str(),
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		1280, 720,
		flags
	);

	if(!m_win){
		throw std::runtime_error(fmt::format("Error in SDL_CreateWindow: {}", SDL_GetError()));
	}
}

engine::client_window::~client_window(){
	if(m_win) SDL_DestroyWindow(m_win);
}

//
// OpenGL window
//

engine::client_window_gl::client_window_gl(const ham_engine_app *app, Uint32 flags)
	: client_window(app, flags | SDL_WINDOW_OPENGL)
	, m_ctx{SDL_GL_CreateContext(window_handle())}
{
	if(!m_ctx){
		throw std::runtime_error(fmt::format("Error in SDL_GL_CreateContext: {}", SDL_GetError()));
	}

	if(SDL_GL_MakeCurrent(window_handle(), m_ctx) != 0){
		SDL_GL_DeleteContext(m_ctx);
		throw std::runtime_error(fmt::format("Error in SDL_GL_MakeCurrent: {}", SDL_GetError()));
	}

	m_frame_data.gl.current_frame = 0;
}

engine::client_window_gl::~client_window_gl(){
	if(m_ctx) SDL_GL_DeleteContext(m_ctx);
}

void engine::client_window_gl::present(f64 dt, ham_renderer *r) const{
	ham_renderer_frame(r, dt, &m_frame_data);

	SDL_GL_SwapWindow(window_handle());

	++m_frame_data.gl.current_frame;
}
