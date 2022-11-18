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

#include "main_window.hpp"

#include <QSettings>
#include <QResizeEvent>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedLayout>
#include <QPushButton>

namespace editor = ham::engine::editor;

editor::main_window::main_window(class project *project_, QWidget *parent)
	: window(parent)
	, m_proj(project_)
{
	Q_ASSERT(project_);

	setMinimumSize(854, 480);
	setWindowTitle(QString("%1 - %2").arg(tr("Ham"), project_->name()));

	QSettings settings;

	settings.beginGroup("editor");

	QList<QPair<QString, QString>> recent_projs{ { project_->name(), project_->dir().absolutePath() } };
	const int cur_num_recents = settings.beginReadArray("recent-projs");
	for(int i = 0; i < cur_num_recents; i++){
		settings.setArrayIndex(i);

		auto name = settings.value("name").toString();
		auto dir  = settings.value("dir").toString();

		if(name == project_->name() && dir == project_->dir().absolutePath()){
			continue;
		}

		recent_projs.emplaceBack(std::move(name), std::move(dir));
	}

	settings.endArray();

	settings.beginWriteArray("recent-projs");

	for(int i = 0; i < recent_projs.size(); i++){
		const auto &recent_info = recent_projs[i];

		settings.setArrayIndex(i);
		settings.setValue("name", recent_info.first);
		settings.setValue("dir",  recent_info.second);
	}

	settings.endArray();

	settings.endGroup(); // editor

	{
		const auto dir_u8 = project_->dir().path().toUtf8();
		const auto author_u8 = project_->author().toUtf8();
		const auto desc_u8 = project_->description().toUtf8();
		const auto name_u8 = project_->name().toUtf8();
		const auto display_u8 = project_->display_name().toUtf8();
		const auto license_u8 = project_->license().toUtf8();
		const auto version = project_->version();

		ham_engine_app engine_app{};

		engine_app.init = [](auto...){ return true; };
		engine_app.fini = [](auto...){};
		engine_app.loop = [](auto...){};

		engine_app.id = project_->id();
		engine_app.dir = ham::str8(dir_u8.data());
		engine_app.name = ham::str8(name_u8.data());
		engine_app.display_name = ham::str8(display_u8.data());
		engine_app.author = ham::str8(author_u8.data());
		engine_app.license = ham::str8(license_u8.data());
		engine_app.description = ham::str8(desc_u8.data());
		engine_app.version = { (u16)version.majorVersion(), (u16)version.minorVersion(), (u16)version.microVersion() };

		m_engine = ham_engine_create(&engine_app);
		if(!m_engine){
			ham::logapierror("Error in ham_engine_create");
			throw std::runtime_error("Could not create main window engine instance");
		}
	}

	m_world = ham_world_create(HAM_LIT("ham-editor"));
	if(!m_world){
		ham::logapierror("Error in ham_world_create");
		ham_engine_destroy(m_engine);
		throw std::runtime_error("Could not create main window world instance");
	}

	m_world_view = new editor::world_view(m_engine, m_world, this);

	const auto inner_widget = new QWidget(this);
	inner_widget->setContentsMargins(0, 0, 0, 0);

	const auto inner_lay = new QStackedLayout(inner_widget);
	inner_lay->setContentsMargins(0, 0, 0, 0);
	inner_lay->setStackingMode(QStackedLayout::StackAll);

//	const auto graph_editor = new editor::graph_editor("Test graph", inner_widget);

//	inner_lay->addWidget(graph_editor);
	inner_lay->addWidget(m_world_view);

	inner_widget->setLayout(inner_lay);

	project_->setParent(this);

	set_central_widget(inner_widget);

//	const auto test_node = graph_editor->new_node(QPointF{0.f, 0.f}, "Hello, Graph!");

//	ham::typeset_view ts = ham_engine_ts(m_engine);

//	const auto test_in_exec_pin = test_node->new_pin(HAM_GRAPH_NODE_PIN_IN, "Test exec", ham::engine::graph_exec_type(ts));
//	(void)test_in_exec_pin;

//	const auto test_in_pin = test_node->new_pin(HAM_GRAPH_NODE_PIN_IN, "Test input", ts.get("i32"));
//	(void)test_in_pin;

//	const auto test_out_pin = test_node->new_pin(HAM_GRAPH_NODE_PIN_OUT, "Test output", ham::engine::graph_exec_type(ts));
//	(void)test_out_pin;

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

	settings.beginGroup("editor");
	const auto sizeVar = settings.value("size");
	const auto maximizedVar = settings.value("maximized");
	settings.endGroup();

	if(sizeVar.isValid()){
		resize(sizeVar.value<QSize>());
	}

	if(maximizedVar.isValid() && maximizedVar.value<bool>()){
		showMaximized();
	}
}

editor::main_window::~main_window(){
	ham_world_destroy(m_world);
	ham_engine_destroy(m_engine);
}

void editor::main_window::changeEvent(QEvent *event){
	window::changeEvent(event);

	if(event->type() != QEvent::WindowStateChange) return;

	QSettings settings;

	switch(windowState()){
		case Qt::WindowState::WindowMaximized:{
			settings.setValue("editor/maximized", true);
			break;
		}

		case Qt::WindowState::WindowMinimized:{
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
