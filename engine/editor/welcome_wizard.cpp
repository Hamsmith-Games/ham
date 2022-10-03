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

#include "welcome_wizard.hpp"

#include <QSettings>

#include <QVBoxLayout>
#include <QHBoxLayout>

#include <QStackedWidget>
#include <QLabel>
#include <QPushButton>
#include <QButtonGroup>
#include <QGroupBox>

namespace editor = ham::engine::editor;

//
// Welcome splash (can be set to skip)
//

editor::welcome_splash::welcome_splash(QWidget *parent)
	: QWizardPage(parent)
{
	QImage logo_img(":/images/logo.png");

	setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

	const auto logo_lbl = new QLabel;

	logo_lbl->setPixmap(QPixmap::fromImage(logo_img));

	const auto msg_lbl = new QLabel(tr("Welcome to the Ham World Engine Editor!"));

	const auto layout = new QVBoxLayout;
	layout->addWidget(logo_lbl, 0, Qt::AlignHCenter | Qt::AlignBottom);
	layout->addWidget(msg_lbl, 0, Qt::AlignCenter);
	setLayout(layout);
}

editor::welcome_splash::~welcome_splash(){}

//
// Welcome wizard
//

editor::welcome_wizard::welcome_wizard(QWidget *parent)
	: QWizard(parent)
{
	setMinimumSize(854, 480);
	setWindowTitle(tr("Welcome!"));

	QSettings settings;
	const bool skip_splash = settings.value("welcome/skip_splash", false).toBool();

	if(!skip_splash){
		addPage(new editor::welcome_splash);
	}
}

editor::welcome_wizard::~welcome_wizard(){}

//
// Welcome window
//

editor::welcome_window::welcome_window(QWidget *parent)
	: window(parent)
{
	setWindowTitle("Welcome!");
	setMinimumSize(854, 480);

	const auto logo_lbl = new QLabel;

	QImage logo_img(":/images/logo.png");
	logo_lbl->setPixmap(QPixmap::fromImage(logo_img));

	const auto projs_box = new QGroupBox(tr("Projects"));

	const auto new_proj_btn  = new QPushButton("New Project");
	connect(new_proj_btn, &QPushButton::pressed, this, &welcome_window::new_proj_pressed);

	const auto open_proj_btn = new QPushButton("Open Project");
	connect(open_proj_btn, &QPushButton::pressed, this, &welcome_window::open_proj_pressed);

	const auto proj_btns_layout = new QHBoxLayout;

	proj_btns_layout->addWidget(new_proj_btn);
	proj_btns_layout->addWidget(open_proj_btn);

	projs_box->setLayout(proj_btns_layout);

	const auto layout = new QVBoxLayout;
	layout->addWidget(logo_lbl, 0, Qt::AlignCenter);
	layout->addWidget(projs_box);

	const auto inner = new QWidget;
	inner->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	inner->setLayout(layout);

	set_central_widget(inner);
}

editor::welcome_window::~welcome_window(){}

void editor::welcome_window::new_proj_pressed(){}

void editor::welcome_window::open_proj_pressed(){}
