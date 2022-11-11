/*
 * Ham World Engine Editor
 * Copyright (C) 2022 Keith Hammond
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

#include "ham/engine.h"
#include "ham/engine/world.h"

#include <QWidget>
#include <QMenu>

#include "renderer_widget.hpp"

Q_DECLARE_METATYPE(ham::engine::world_view)

namespace ham::engine::editor{
	class world_view;

	class world_context_menu: public QMenu{
		Q_OBJECT

		Q_PROPERTY(ham::engine::editor::world_view* world_view READ world_view CONSTANT)

		public:
			explicit world_context_menu(class world_view *world_view_, QWidget *parent = nullptr);
			~world_context_menu();

			class world_view *world_view() noexcept{ return m_view; }
			const class world_view *world_view() const noexcept{ return m_view; }

		private:
			class world_view *m_view;
	};

	class world_view: public QWidget{
		Q_OBJECT

		Q_PROPERTY(ham::engine::editor::renderer_widget* renderer READ renderer CONSTANT)
		Q_PROPERTY(ham::engine::world_view world READ world CONSTANT)

		public:
			world_view(ham_engine *engine, ham_world *world, QWidget *parent = nullptr);
			~world_view();

			renderer_widget *renderer() const noexcept{ return m_r_widget; }

			ham::engine::world_view world() const noexcept{ return m_world; }

			void show_context_menu(const QPoint &pos);

			bool eventFilter(QObject *watched, QEvent *ev) override;

		protected:
			void mouseMoveEvent(QMouseEvent *ev) override;
			void mousePressEvent(QMouseEvent *ev) override;
			void mouseReleaseEvent(QMouseEvent *ev) override;

			void keyPressEvent(QKeyEvent *ev) override;
			void keyReleaseEvent(QKeyEvent *ev) override;

			void resizeEvent(QResizeEvent *ev) override;

		private:
			ham_engine *m_engine;
			ham_world *m_world;
			renderer_widget *m_r_widget;
			draw_group m_gizmo_group, m_test_group;

			ham_light *m_cam_light;
			light_group	m_cam_light_group;

			ham::camera m_cam;
			Qt::MouseButton m_cam_btn = Qt::MiddleButton;
			bool m_cam_held = false;
			QPointF m_cam_last_pos;
			ham::vec3 m_cam_dir;
	};
}

#endif // !HAM_ENGINE_EDITOR_WORLD_VIEW_HPP
