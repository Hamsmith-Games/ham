/*
 * Ham World Engine Editor
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

#include "world_view.hpp"

#include <QOpenGLContext>
#include <QHBoxLayout>

namespace editor = ham::engine::editor;

//
// OpenGL widget
//

static QOpenGLContext *ham_impl_editor_gl_current_context = nullptr;

editor::detail::opengl_widget::opengl_widget(QWidget *parent)
	: QOpenGLWidget(parent)
{
	setContentsMargins(0, 0, 0, 0);
	m_frame_data.gl.current_frame = 0;
}

editor::detail::opengl_widget::~opengl_widget(){
	ham_renderer_destroy(m_r);
}

static inline void *ham_impl_engine_editor_glGetProcAddr(const char *name){
	return (void*)ham_impl_editor_gl_current_context->getProcAddress(name);
}

void ham::engine::editor::detail::opengl_widget::initializeGL(){
	makeCurrent();

	ham_impl_editor_gl_current_context = context();

	ham_renderer_create_args create_args;

	create_args.gl = {
		.context_handle = (uptr)ham_impl_editor_gl_current_context,
		.glGetProcAddr  = ham_impl_engine_editor_glGetProcAddr,
	};

	m_r = ham_renderer_create(HAM_RENDERER_GL_PLUGIN_NAME, HAM_RENDERER_GL_OBJECT_NAME, &create_args);
	if(!m_r){
		throw std::runtime_error("Error in ham_renderer_create");
	}

	ham_renderer_resize(m_r, width(), height());

	ham_ticker_reset(&m_ticker);
}

void ham::engine::editor::detail::opengl_widget::resizeGL(int w, int h){
	makeCurrent();

	ham_renderer_resize(m_r, w, h);
}

void ham::engine::editor::detail::opengl_widget::paintGL(){
	f64 dt = ham_ticker_tick(&m_ticker, 0.0);

	ham_renderer_frame(m_r, dt, &m_frame_data);

	update();

	++m_frame_data.gl.current_frame;
}

//
// World editor main view
//

editor::world_view::world_view(QWidget *parent)
	: QWidget(parent)
{
	setContentsMargins(0, 0, 0, 0);

	m_r_widget = new editor::detail::opengl_widget(this);

	const auto lay = new QHBoxLayout(this);

	lay->setContentsMargins(0, 0, 0, 0);
	lay->addWidget(m_r_widget);

	setLayout(lay);
}

editor::world_view::~world_view(){}
