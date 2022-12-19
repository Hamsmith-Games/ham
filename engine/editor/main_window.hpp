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

#ifndef HAM_ENGINE_EDITOR_MAIN_WINDOW_HPP
#define HAM_ENGINE_EDITOR_MAIN_WINDOW_HPP 1

#include "ham/engine/world.h"

#include "window.hpp"

namespace ham::engine::editor{
	class project;
	class world_view;
	class graph_editor;
	class source_editor;

	class physics_backend;
	class engine_instance;
}

namespace ham::engine::editor{
	class main_window: public window{
		Q_OBJECT

		public:
			explicit main_window(class project *project_, QWidget *parent = nullptr);
			~main_window();

			engine_instance *engine() const noexcept{ return m_engine; }
			class project *project() const noexcept{ return m_proj; }
			class world_view *world_view() noexcept{ return m_world_view; }

			void show_graph_editor(bool do_show = true);
			void hide_graph_editor(){ show_graph_editor(false); }

		protected:
			void changeEvent(QEvent *event) override;
			void resizeEvent(QResizeEvent *ev) override;

			void keyReleaseEvent(QKeyEvent *event) override;

		private:
			engine_instance *m_engine;
			physics_backend *m_phys;
			ham_world *m_world;

			class project *m_proj;
			class world_view *m_world_view;
			class graph_editor *m_graph_editor;
			class source_editor *m_source_editor;
			QWidget *m_view_overlay;
	};
}

#endif // !HAM_ENGINE_EDITOR_MAIN_WINDOW_HPP
