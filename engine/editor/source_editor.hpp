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

#ifndef HAM_ENGINE_EDITOR_SOURCE_EDITOR_HPP
#define HAM_ENGINE_EDITOR_SOURCE_EDITOR_HPP 1

#include "Qsci/qsciscintilla.h"

namespace ham::engine::editor{
	class source_editor: public QsciScintilla{
		public:
			explicit source_editor(QWidget *parent = nullptr);
			~source_editor();
	};
}

#endif // !HAM_ENGINE_EDITOR_SOURCE_EDITOR_HPP
