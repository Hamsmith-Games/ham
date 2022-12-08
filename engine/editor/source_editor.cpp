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

#include "source_editor.hpp"

#include <QFontDatabase>

namespace editor = ham::engine::editor;

editor::source_editor::source_editor(QWidget *parent)
	: QsciScintilla(parent)
{
	setCaretForegroundColor(QColor(255, 255, 255));
	setCaretLineBackgroundColor(QColor(66, 66, 66));
	setCaretLineVisible(true);

	setMarginType(0, QsciScintilla::NumberMargin);
	setMarginWidth(0, "0000");

	setMarginType(1, QsciScintilla::SymbolMargin);

	setMarginsForegroundColor(QColor(255, 255, 255));
	setMarginsBackgroundColor(QColor(105, 105, 105));

	setIndentationsUseTabs(true);
	setTabWidth(4);
	setTabIndents(true);
	setAutoIndent(true);

	setText("Hello, World!");
	setUtf8(true);

	setFont(QFontDatabase::font("JetBrains Mono", "Regular", 12));
}

editor::source_editor::~source_editor(){}
