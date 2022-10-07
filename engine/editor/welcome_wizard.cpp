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
#include "main_window.hpp"

#include <QSettings>

#include <QVBoxLayout>
#include <QHBoxLayout>

#include <QMessageBox>
#include <QLabel>
#include <QCheckBox>
#include <QPushButton>
#include <QButtonGroup>
#include <QGroupBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QListWidget>
#include <QSplitter>
#include <QFileDialog>
#include <QStandardPaths>

namespace editor = ham::engine::editor;

//
// Project Wizard
//

// initial page

editor::project_wizard::initial_page::initial_page(QWidget *parent)
	: QWizardPage(parent)
{
	setTitle(tr("Project Info"));
	setSubTitle(tr("It all starts here."));

	const auto name_lbl = new QLabel(tr("Name"));
	const auto name_input = new QLineEdit;
	name_lbl->setBuddy(name_input);

	const auto dir_lbl = new QLabel(tr("New Directory"));
	const auto dir_line = new QLineEdit;
	const auto dir_btn = new QPushButton;
	dir_lbl->setBuddy(dir_line);

	const auto projs_dir = editor::default_project_path();
	if(!QDir(projs_dir).exists()){
		qWarning() << "Project directory does not exist" << projs_dir;
		QMessageBox::warning(this, "Error", QString("Default project directory does not exist: %1").arg(projs_dir));
	}
	else{
		dir_line->setText(projs_dir);
	}

	const auto subdir_lbl   = new QLabel(tr("Create subdirectory"));
	const auto subdir_check = new QCheckBox;
	subdir_lbl->setBuddy(subdir_check);
	subdir_check->setCheckState(Qt::Checked);

	connect(dir_btn, &QPushButton::pressed, this, [this, dir_line]{
		const auto dir_path = QFileDialog::getExistingDirectory(this, "Project directory", dir_line->text());
		if(!dir_path.isEmpty()){
			dir_line->setText(dir_path);
		}
	});

	const auto dir_open_img = QImage(":/images/folder-open.png");
	const auto dir_open_icon = QIcon(QPixmap::fromImage(dir_open_img).scaledToWidth(32));

	dir_btn->setIcon(dir_open_icon);

	dir_line->setReadOnly(true);

	const auto dir_line_lay = new QHBoxLayout;

	dir_line_lay->addWidget(dir_line);
	dir_line_lay->addWidget(dir_btn);

	registerField("name*", name_input);
	registerField("dir", dir_line);
	registerField("subdir", subdir_check);

	const auto layout = new QGridLayout;

	layout->addWidget(name_lbl, 0, 0);
	layout->addWidget(name_input, 0, 1);

	layout->addWidget(dir_lbl, 1, 0);
	layout->addLayout(dir_line_lay, 1, 1);

	layout->addWidget(subdir_lbl, 2, 0);
	layout->addWidget(subdir_check, 2, 1);

	setLayout(layout);
}

editor::project_wizard::initial_page::~initial_page(){}

bool editor::project_wizard::initial_page::validatePage(){
	const auto name_str = field("name").toString();
	const auto name_utf8 = name_str.toUtf8();

	if(!name_utf8.isValidUtf8()) return false;

	const auto name_str_beg = name_utf8.begin();
	const auto name_str_end = name_utf8.end();
	auto name_str_it = name_str_beg;

	if(!std::isalpha(*name_str_it)){
		return false;
	}

	++name_str_it;

	for(; name_str_it != name_str_end; ++name_str_it){
		const char ch = *name_str_it;
		if(std::isspace(ch) || (!std::isalnum(ch) && ch != '_' && ch != '-')){
			return false;
		}
	}


	auto dir_str = field("dir").toString();
	if(!QDir(dir_str).exists()){
		qDebug() << "Directory does not exist" << dir_str;
		return false;
	}

	const auto is_subdir = field("subdir").toBool();
	if(is_subdir){
		dir_str += QString("/%1").arg(name_str);
		if(!QDir(dir_str).exists()){
			return QWizardPage::validatePage();
		}
	}

	if(!QDir(dir_str).entryList(QDir::NoDotAndDotDot|QDir::AllEntries).isEmpty()){
		qDebug() << "Directory is not empty" << dir_str;
		return false;
	}

	return QWizardPage::validatePage();
}

// info page

editor::project_wizard::info_page::info_page(QWidget *parent)
	: QWizardPage(parent)
{
	setTitle(tr("Basic Info"));
	setSubTitle(tr("Cross the t's and dot the i's."));

	const auto display_lbl = new QLabel(tr("Display Name"));
	const auto display_input = new QLineEdit;
	display_lbl->setBuddy(display_input);

	const auto author_lbl = new QLabel(tr("Author"));
	const auto author_input = new QLineEdit;
	author_lbl->setBuddy(author_input);

	const auto desc_lbl = new QLabel(tr("Description"));
	const auto desc_input = new QTextEdit;
	desc_lbl->setBuddy(desc_input);

	this->registerField("display*", display_input);
	this->registerField("author*", author_input);
	this->registerField("desc", desc_input);

	const auto layout = new QGridLayout;

	layout->addWidget(display_lbl, 0, 0);
	layout->addWidget(display_input, 0, 1);

	layout->addWidget(author_lbl, 1, 0);
	layout->addWidget(author_input, 1, 1);

	layout->addWidget(desc_lbl, 2, 0, 1, 2);
	layout->addWidget(desc_input, 3, 0, 1, 2);

	setLayout(layout);
}

editor::project_wizard::info_page::~info_page(){}

bool editor::project_wizard::info_page::validatePage(){
	return QWizardPage::validatePage();
}

// template page

editor::project_wizard::template_page::template_page(QWidget *parent)
	: QWizardPage(parent)
{
	setTitle(tr("Template"));
	setSubTitle(tr("Pre-cooked meals."));
	setContentsMargins(0, 0, 0, 0);

	const auto template_list = new QListView;

	const auto template_model = new class template_model;

	QList<editor::project_template*> tmpls;

	const auto template_dirs = QDir(":/templates").entryList(QDir::NoDotAndDotDot | QDir::AllDirs);

	for(auto &&template_subdir : template_dirs){
		const auto tmpl_dir = QDir(":/templates/" + template_subdir);
		const auto json_path = tmpl_dir.path() + "/ham-engine-template.json";

		qDebug() << "Checking for template JSON:" << json_path;

		if(!QFileInfo::exists(json_path)){
			continue;
		}

		const auto tmpl = new editor::project_template(tmpl_dir.absolutePath(), template_list);

		qDebug() << "Found template:" << tmpl->name();

		tmpls.append(tmpl);
	}

	if(tmpls.empty()){
		QMessageBox::warning(this, "Error", "No project templates found.");
		wizard()->setResult(-1);
		this->close();
	}

	m_cur_tmpl = tmpls.at(0);

	template_model->set_templates(tmpls);
	template_list->setModel(template_model);

	const auto preview_title = new QLabel;
	preview_title->setTextFormat(Qt::TextFormat::PlainText);

	const auto preview_desc = new QTextEdit;
	preview_desc->setReadOnly(true);

	const auto preview_lay = new QVBoxLayout;

	preview_lay->addWidget(preview_title, 0, Qt::AlignTop | Qt::AlignHCenter);
	preview_lay->addWidget(preview_desc, 1);

	const auto preview_widget = new QWidget;
	preview_widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored);
	//preview_widget->setContentsMargins(0, 0, 0, 0);
	preview_widget->setLayout(preview_lay);

	const auto template_splitter = new QSplitter;
	//template_splitter->setSizeIncrement(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	template_splitter->addWidget(template_list);
	template_splitter->addWidget(preview_widget);

	connect(template_list, &QListView::activated, this, [this, tmpls, template_model, preview_title, preview_desc](const QModelIndex &index){
		const auto tmpl_name   = template_model->data(index, template_model::NameRole).toString();
		const auto tmpl_dir    = template_model->data(index, template_model::DirRole).toString();
		//const auto tmpl_author = template_model->data(index, template_model::AuthorRole);
		const auto tmpl_desc   = template_model->data(index, template_model::DescriptionRole).toString();

		preview_title->setText(tmpl_name);
		preview_desc->setMarkdown(tmpl_desc);

		m_cur_tmpl = tmpls.at(index.row());
	});

	const auto layout = new QVBoxLayout;
	layout->setContentsMargins(0, 0, 0, 0);

	layout->addWidget(template_splitter, 1);

	setLayout(layout);
}

editor::project_wizard::template_page::~template_page(){}

bool editor::project_wizard::template_page::validatePage(){
	const auto template_dir = field("template_dir").toString();
	if(!QDir(template_dir).exists()){
		return false;
	}

	return QWizardPage::validatePage();
}

// wizard itself

editor::project_wizard::project_wizard(QWidget *parent)
	: QWizard(parent)
{
	setWindowTitle(tr("New Project"));

	setWizardStyle(QWizard::ModernStyle);

	//QImage logo_img(":/images/logo.png");
	//setPixmap(QWizard::LogoPixmap, QPixmap::fromImage(logo_img).scaledToWidth(128));

	const auto tmpl_page = new template_page;

	addPage(new initial_page);
	addPage(new info_page);
	addPage(tmpl_page);

	connect(this, &QDialog::accepted, this, [this, tmpl_page]{
		const auto is_subdir     = field("subdir").toBool();
		const auto proj_name     = field("name").toString();
		auto proj_dir            = field("dir").toString();
		const auto proj_display  = field("display").toString();
		const auto proj_author   = field("author").toString();
		const auto proj_desc     = field("desc").toString();

		if(is_subdir){
			proj_dir = QString("%1/%2").arg(proj_dir, proj_name);

			if(!QDir().mkdir(proj_dir)){
				QMessageBox::warning(this, "Error", QString("Failed to create directory \"%1\"").arg(proj_dir));
				QWizard::done(-1);
				return;
			}

			setField("dir", proj_dir);
			setField("subdir", false);
		}

		const auto proj_tmpl = tmpl_page->current_template();
		if(!proj_tmpl->createInDir(QDir(proj_dir), proj_name, proj_display, proj_author, proj_desc)){
			QMessageBox::warning(this, "Error", QString("Failed to create project \"%2\" in \"%1\"").arg(proj_dir, proj_name));
			QWizard::done(-1);
			return;
		}
	});

	// TODO: fix weird button focus while first typing bug
}

editor::project_wizard::~project_wizard(){}

//
// Welcome window
//

editor::welcome_window::welcome_window(QWidget *parent)
	: window(parent)
	, m_splash(createWelcomeSplash())
{
	setWindowTitle(tr("Welcome!"));
	setMinimumSize(854, 480);

	set_central_widget(m_splash);
}

editor::welcome_window::~welcome_window(){}

void editor::welcome_window::new_proj_pressed(){
	setWindowTitle(tr("New Project"));

	const auto proj_wizard = new editor::project_wizard;
	set_central_widget(proj_wizard);

	connect(proj_wizard, &QWizard::accepted, this, [this, proj_wizard]{
		const auto proj_dir = proj_wizard->field("dir").toString();
		const auto proj = new editor::project(QDir(proj_dir));
		const auto main_win = new editor::main_window(proj);
		main_win->show();
		this->close();
		/*
		{
			QMessageBox::warning(this, "Error creating project", "An error ocurred while creating the project.");
			setWindowTitle(tr("Welcome!"));
			set_central_widget(m_splash);
		}
		*/
	});

	connect(proj_wizard, &QWizard::rejected, this, [this]{
		setWindowTitle(tr("Welcome!"));
		set_central_widget(m_splash);
	});
}

void editor::welcome_window::open_proj_pressed(){
	const auto proj_dir = QFileDialog::getExistingDirectory(this, tr("Project Directory"), editor::default_project_path());
	if(proj_dir.isEmpty()) return;

	const auto proj = new editor::project(QDir(proj_dir));
	const auto main_win = new editor::main_window(proj);
	main_win->show();
	this->close();
}

QWidget *editor::welcome_window::createWelcomeSplash(){
	const auto logo_lbl = new QLabel;

	QImage logo_img(":/images/logo.png");
	logo_lbl->setPixmap(QPixmap::fromImage(logo_img));

	const auto projs_box = new QGroupBox(tr("Projects"));
	projs_box->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

	const auto new_proj_btn  = new QPushButton(tr("New"));
	connect(new_proj_btn, &QPushButton::pressed, this, &welcome_window::new_proj_pressed);

	const auto open_proj_btn = new QPushButton(tr("Open"));
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
