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

#include "main_window.hpp"

#include <QVBoxLayout>

namespace editor = ham::engine::editor;

editor::main_window::main_window(class project *project_, QWidget *parent)
	: window(parent)
	, m_proj(project_)
	, m_world_view(new editor::world_view(this))
{
	setMinimumSize(854, 480);
	setWindowTitle(QString("%1 - %2").arg(tr("Ham"), project_->name()));

	project_->setParent(this);

	const auto inner = new QWidget;
	inner->setContentsMargins(0, 0, 0, 0);

	const auto layout = new QVBoxLayout;
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(m_world_view, 1);

	inner->setLayout(layout);

	set_central_widget(inner);
}

editor::main_window::~main_window(){}
