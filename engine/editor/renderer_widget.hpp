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

#ifndef HAM_ENGINE_EDITOR_RENDERER_WIDGET_HPP
#define HAM_ENGINE_EDITOR_RENDERER_WIDGET_HPP 1

#include <QWidget>
#include <QMetaType>

#include "ham/renderer-object.h"
#include "ham/time.h"

class QVulkanInstance;

Q_DECLARE_METATYPE(ham_renderer*)

namespace ham::engine::editor{
	class renderer_widget: public QWidget{
		Q_OBJECT
		Q_PROPERTY(ham_renderer* renderer READ renderer CONSTANT)
		Q_PROPERTY(ham_renderer_frame_data frame_data READ frame_data CONSTANT)

		public:
			virtual ~renderer_widget();

			ham_renderer *renderer() const noexcept{ return m_r; }

			ham_renderer_frame_data &frame_data() noexcept{ return m_frame_data; }
			const ham_renderer_frame_data &frame_data() const noexcept{ return m_frame_data; }

		public Q_SLOTS:
			bool initialize_renderer(const char *plugin_id, const char *obj_id, const ham_renderer_create_args *args);
			void finalize_renderer();

			void resize_renderer(u32 w, u32 h);

			void paint_renderer();

		protected:
			explicit renderer_widget(ham_renderer *r = nullptr, QWidget *parent = nullptr);
			explicit renderer_widget(QWidget *parent = nullptr): renderer_widget(nullptr, parent){}

		private:
			ham_renderer *m_r;
			ham_ticker m_ticker;
			ham_renderer_frame_data m_frame_data;
	};

	class renderer_widget_gl: public renderer_widget{
		Q_OBJECT

		public:
			explicit renderer_widget_gl(QWidget *parent = nullptr);

			~renderer_widget_gl();
	};

	class renderer_widget_vulkan: public renderer_widget{
		Q_OBJECT

		public:
			explicit renderer_widget_vulkan(QWidget *parent = nullptr);

			~renderer_widget_vulkan();

		protected:
			void paintEvent(QPaintEvent *ev) override;

		private:
			QVulkanInstance *m_inst;
			QWindow *m_win;
	};
}

#endif // !HAM_ENGINE_EDITOR_RENDERER_WIDGET_HPP
