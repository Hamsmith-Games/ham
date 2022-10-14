/*
 * Ham World Engine Editor
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

#include "renderer_widget.hpp"

#include "ham/log.h"

#include <QBoxLayout>
#include <QOpenGLWidget>
#include <QOpenGLContext>

using namespace ham::typedefs;

namespace editor = ham::engine::editor;

static thread_local QOpenGLContext *ham_impl_editor_gl_current_context = nullptr;

static inline void *ham_impl_engine_editor_glGetProcAddr(const char *name){
	return (void*)ham_impl_editor_gl_current_context->getProcAddress(name);
}

namespace ham::engine::editor::detail{
	class renderer_opengl_widget: public QOpenGLWidget{
		public:
			renderer_opengl_widget(renderer_widget_gl *parent)
				: QOpenGLWidget(parent)
				, m_r(parent)
			{
				Q_ASSERT(parent != nullptr);

				setContentsMargins(0, 0, 0, 0);
			}

		protected:
			void initializeGL() override{
				makeCurrent();

				ham_impl_editor_gl_current_context = context();

				ham_renderer_create_args create_args;

				create_args.gl = {
					.context_handle = std::bit_cast<uptr>(ham_impl_editor_gl_current_context),
					.glGetProcAddr  = ham_impl_engine_editor_glGetProcAddr,
				};

				if(!m_r->initialize_renderer(HAM_RENDERER_GL_PLUGIN_NAME, HAM_RENDERER_GL_OBJECT_NAME, &create_args)){
					throw std::runtime_error("Error in ham_renderer_create");
				}

				m_r->resize_renderer(width(), height());
			}

			void resizeGL(int w, int h) override{
				makeCurrent();
				m_r->resize_renderer((u32)w, (u32)h);
			}

			void paintGL() override{
				m_r->paint_renderer();
				update();
			}

		private:
			renderer_widget_gl *m_r;
	};
}

editor::renderer_widget_gl::renderer_widget_gl(QWidget *parent)
	: renderer_widget(parent)
{
	const auto glwidget = new detail::renderer_opengl_widget(this);

	const auto lay = new QBoxLayout(QBoxLayout::LeftToRight, this);

	lay->setContentsMargins(0, 0, 0, 0);
	lay->addWidget(glwidget);

	setLayout(lay);
}

editor::renderer_widget_gl::~renderer_widget_gl(){}
