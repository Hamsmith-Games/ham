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

#ifndef HAM_ENGINE_EDITOR_MAIN_WINDOW_HPP
#define HAM_ENGINE_EDITOR_MAIN_WINDOW_HPP 1

#include <QMainWindow>

namespace ham::engine::editor{
	class main_window: public QMainWindow{
		Q_OBJECT

		public:
			main_window();
			~main_window();
	};
}

#endif // !HAM_ENGINE_EDITOR_MAIN_WINDOW_HPP
