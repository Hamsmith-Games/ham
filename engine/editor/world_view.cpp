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

#include "world_view.hpp"

#include <QHBoxLayout>

namespace editor = ham::engine::editor;

//
// World editor main view
//

editor::world_view::world_view(QWidget *parent)
	: QWidget(parent)
{
	setContentsMargins(0, 0, 0, 0);

	m_r_widget = new editor::renderer_widget_gl(this);

	const auto lay = new QHBoxLayout(this);

	lay->setContentsMargins(0, 0, 0, 0);
	lay->addWidget(m_r_widget);

	setLayout(lay);
}

editor::world_view::~world_view(){}
