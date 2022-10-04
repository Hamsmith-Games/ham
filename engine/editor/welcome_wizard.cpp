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
#include <QLineEdit>
#include <QTextEdit>
#include <QFileDialog>

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
// Project Wizard
//

// initial page

editor::project_wizard::initial_page::initial_page(QWidget *parent)
	: QWizardPage(parent)
{
	setTitle("Project Info");
	setSubTitle("It all starts here.");

	const auto name_lbl = new QLabel("Name");
	const auto name_input = new QLineEdit;

	const auto dir_lbl = new QLabel("New Directory");
	const auto dir_btn = new QPushButton;
	const auto dir_line = new QLineEdit;

	connect(dir_btn, &QPushButton::pressed, this, [dir_line]{
		const auto dir_path = QFileDialog::getExistingDirectory();
		dir_line->setText(dir_path);
	});

	const auto dir_open_img = QImage(":/images/folder-open.png");
	const auto dir_open_icon = QIcon(QPixmap::fromImage(dir_open_img).scaledToWidth(32));

	dir_btn->setIcon(dir_open_icon);

	dir_line->setReadOnly(true);

	const auto dir_line_lay = new QHBoxLayout;

	dir_line_lay->addWidget(dir_line);
	dir_line_lay->addWidget(dir_btn);

	registerField("name*", name_input);
	registerField("dir*", dir_line);

	const auto layout = new QGridLayout;

	layout->addWidget(name_lbl, 0, 0);
	layout->addWidget(name_input, 0, 1);

	layout->addWidget(dir_lbl, 1, 0);
	layout->addLayout(dir_line_lay, 1, 1);

	setLayout(layout);
}

editor::project_wizard::initial_page::~initial_page(){}

bool editor::project_wizard::initial_page::validatePage(){
	return QWizardPage::validatePage();
}

// info page

editor::project_wizard::info_page::info_page(QWidget *parent)
	: QWizardPage(parent)
{
	setTitle("Basic Info");
	setSubTitle("Cross the t's and dot the i's.");

	const auto name_lbl = new QLabel("Name");
	const auto display_lbl = new QLabel("Display Name");
	const auto author_lbl = new QLabel("Author");
	const auto desc_lbl = new QLabel("Description");

	const auto name_input = new QLineEdit;
	const auto display_input = new QLineEdit;
	const auto author_input = new QLineEdit;
	const auto desc_input = new QTextEdit;

	this->registerField("name*", name_input);
	this->registerField("display*", display_input);
	this->registerField("author*", author_input);
	this->registerField("desc", desc_input);

	const auto layout = new QGridLayout;

	layout->addWidget(name_lbl, 0, 0);
	layout->addWidget(name_input, 0, 1);

	layout->addWidget(display_lbl, 1, 0);
	layout->addWidget(display_input, 1, 1);

	layout->addWidget(author_lbl, 2, 0);
	layout->addWidget(author_input, 2, 1);

	layout->addWidget(desc_lbl, 3, 0, 1, 2);
	layout->addWidget(desc_input, 4, 0, 1, 2);

	setLayout(layout);
}

editor::project_wizard::info_page::~info_page(){}

bool editor::project_wizard::info_page::validatePage(){
	return QWizardPage::validatePage();
}

// wizard itself

editor::project_wizard::project_wizard(QWidget *parent)
	: QWizard(parent)
{
	setWindowTitle("New Project");

	addPage(new initial_page);
	addPage(new info_page);
}

editor::project_wizard::~project_wizard(){}

//
// Welcome window
//

editor::welcome_window::welcome_window(QWidget *parent)
	: window(parent)
	, m_splash(createWelcomeSplash())
{
	setWindowTitle("Welcome!");
	setMinimumSize(854, 480);

	set_central_widget(m_splash);
}

editor::welcome_window::~welcome_window(){}

void editor::welcome_window::new_proj_pressed(){
	setWindowTitle("New Project");

	const auto proj_wizard = new editor::project_wizard;
	set_central_widget(proj_wizard);

	connect(proj_wizard, &QWizard::finished, this, [this]{
		setWindowTitle("Welcome!");
		set_central_widget(m_splash);
	});
}

void editor::welcome_window::open_proj_pressed(){
	const auto proj_dir = QFileDialog::getExistingDirectory(this, "Project Directory");
	(void)proj_dir;
}

QWidget *editor::welcome_window::createWelcomeSplash(){
	const auto logo_lbl = new QLabel;

	QImage logo_img(":/images/logo.png");
	logo_lbl->setPixmap(QPixmap::fromImage(logo_img));

	const auto projs_box = new QGroupBox(tr("Projects"));
	projs_box->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

	const auto new_proj_btn  = new QPushButton("New");
	connect(new_proj_btn, &QPushButton::pressed, this, &welcome_window::new_proj_pressed);

	const auto open_proj_btn = new QPushButton("Open");
	connect(open_proj_btn, &QPushButton::pressed, this, &welcome_window::open_proj_pressed);

	const auto proj_btns_layout = new QHBoxLayout;

	proj_btns_layout->addWidget(new_proj_btn);
	proj_btns_layout->addWidget(open_proj_btn);

	projs_box->setLayout(proj_btns_layout);

	const auto layout = new QVBoxLayout;
	layout->addWidget(logo_lbl, 0, Qt::AlignCenter);
	layout->addWidget(projs_box);

	const auto ret = new QWidget;
	ret->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	ret->setLayout(layout);

	return ret;
}
