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

#ifndef HAM_ENGINE_EDITOR_WORLD_VIEW_HPP
#define HAM_ENGINE_EDITOR_WORLD_VIEW_HPP 1

#include "ham/time.h"
#include "ham/renderer.h"

#include <QWidget>

#include <QOpenGLWidget>

#include <QVulkanWindow>
#include <QVulkanDeviceFunctions>

namespace ham::engine::editor{
	namespace detail{
		class vulkan_renderer: public QVulkanWindowRenderer{
			public:
				explicit vulkan_renderer(QVulkanWindow *window_)
					: m_window(window_)
				{
					m_frame_data.current_frame = 0;
				}

				void initResources() override;
				void releaseResources() override;

				void startNextFrame() override;

			private:
				QVulkanWindow *m_window;
				ham_ticker m_ticker;
				ham_renderer *m_r;
				ham_renderer_frame_data m_frame_data;

		};

		class vulkan_window: public QVulkanWindow{
			Q_OBJECT

			public:
				explicit vulkan_window(QWindow *parent = nullptr)
					: QVulkanWindow(parent){}

				vulkan_renderer *createRenderer() override{
					return new vulkan_renderer(this);
				}
		};

		class opengl_widget: public QOpenGLWidget{
			Q_OBJECT

			public:
				explicit opengl_widget(QWidget *parent = nullptr);
				~opengl_widget();

			protected:
				void initializeGL() override;
				void resizeGL(int w, int h) override;
				void paintGL() override;

			private:
				ham_renderer *m_r = nullptr;
				ham_renderer_frame_data m_frame_data;
				ham_ticker m_ticker;
		};
	}

	class world_view: public QWidget{
		Q_OBJECT

		Q_PROPERTY(QWidget* renderer_widget READ renderer_widget CONSTANT)

		public:
			explicit world_view(QWidget *parent = nullptr);
			~world_view();

			QWidget *renderer_widget() const noexcept{ return m_r_widget; }

		private:
			QWidget *m_r_widget;
	};
}

#endif // !HAM_ENGINE_EDITOR_WORLD_VIEW_HPP
