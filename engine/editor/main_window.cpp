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

#include "main_window.hpp"

#include <QSettings>
#include <QResizeEvent>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

namespace editor = ham::engine::editor;

editor::main_window::main_window(class project *project_, QWidget *parent)
	: window(parent)
	, m_proj(project_)
	, m_world_view(new editor::world_view(this))
{
	setMinimumSize(854, 480);
	setWindowTitle(QString("%1 - %2").arg(tr("Ham"), project_->name()));

	project_->setParent(this);

	set_central_widget(m_world_view);

	const auto settings_img = QImage("://images/cog.png").scaledToWidth(32, Qt::SmoothTransformation);
	const auto settings_pix = QPixmap::fromImage(settings_img);

	const auto settings_btn = new QPushButton(this);
	settings_btn->setFixedSize(32, 32);
	settings_btn->setContentsMargins(0, 0, 0, 0);
	settings_btn->setIcon(QIcon(settings_pix));
	settings_btn->setIconSize(QSize(32, 32));

	const auto header_btn_lay = new QHBoxLayout;
	header_btn_lay->setContentsMargins(0, 0, 0, 0);
	header_btn_lay->addWidget(settings_btn, 1, Qt::AlignLeft | Qt::AlignVCenter);

	header()->setGapLayout(window_header::gap::left, header_btn_lay);

	QSettings settings;

	const auto sizeVar = settings.value("editor/size");
	const auto maximizedVar = settings.value("editor/maximized");

	if(sizeVar.isValid()){
		resize(sizeVar.value<QSize>());
	}

	if(maximizedVar.isValid() && maximizedVar.value<bool>()){
		showMaximized();
	}
}

editor::main_window::~main_window(){}

void editor::main_window::changeEvent(QEvent *event){
	window::changeEvent(event);

	if(event->type() != QEvent::WindowStateChange) return;

	QSettings settings;

	switch(windowState()){
		case Qt::WindowState::WindowMaximized:{
			settings.setValue("editor/maximized", true);
			break;
		}

		default:{
			settings.setValue("editor/maximized", false);
			break;
		}
	}
}

void editor::main_window::resizeEvent(QResizeEvent *ev){
	window::resizeEvent(ev);

	QSettings settings;
	settings.setValue("editor/size", ev->size());
}
