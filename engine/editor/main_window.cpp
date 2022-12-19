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
#include "project.hpp"
#include "engine_instance.hpp"
#include "world_view.hpp"
#include "graph_editor.hpp"
#include "source_editor.hpp"
#include "physics_backend.hpp"

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
	setWindowTitle(QString("%1 - %2").arg(tr("Ham"), project_->name().toString()));

	QSettings settings;

	settings.beginGroup("editor");

	QList<QPair<QUtf8StringView, QString>> recent_projs{ { project_->name(), project_->dir().absolutePath() } };
	const int cur_num_recents = settings.beginReadArray("recent-projs");
	for(int i = 0; i < cur_num_recents; i++){
		settings.setArrayIndex(i);

		auto name = settings.value("name").toString().toUtf8();
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
		settings.setValue("name", recent_info.first.toString());
		settings.setValue("dir",  recent_info.second);
	}

	settings.endArray();

	settings.endGroup(); // editor

	{
		const auto app = new editor::engine_app(
			project_->id(),
			project_->dir(),
			project_->name(),
			project_->display_name(),
			project_->author(),
			project_->license(),
			project_->description(),
			project_->version(),
			[](auto...){ return true; },
			[](auto...){},
			[](auto...){},
			nullptr
		);

		m_engine = new editor::engine_instance(app, project_->ts(), this);
		if(!m_engine){
			throw std::runtime_error("Could not create main window engine instance");
		}
	}

	m_phys = new editor::physics_backend(this);
	if(!m_phys){
		delete m_engine;
		throw std::runtime_error("Could not create main window world instance");
	}

	m_world = ham_world_create(HAM_LIT("ham-editor"), m_phys->get());
	if(!m_world){
		delete m_phys;
		delete m_engine;
		throw std::runtime_error("Could not create main window world instance");
	}

	m_world_view = new editor::world_view(m_engine->handle(), m_world, this);

	const auto inner_widget = new QWidget(this);
	inner_widget->setContentsMargins(0, 0, 0, 0);

	const auto inner_lay = new QStackedLayout(inner_widget);
	inner_lay->setContentsMargins(0, 0, 0, 0);
	inner_lay->setStackingMode(QStackedLayout::StackAll);

	ham::typeset_view ts = project_->ts();

	const auto proj_name_str = project_->name().toString();

	const auto app_graph = project_->get_graph(proj_name_str);
	if(app_graph){
		m_graph_editor = new editor::graph_editor(app_graph, inner_widget);
	}
	else{
		m_graph_editor = new editor::graph_editor(proj_name_str, ts, inner_widget);

		const auto test_node = m_graph_editor->new_node(QPointF{0.f, 0.f}, "Hello, Graph!");

		const auto test_in_exec_pin = test_node->new_pin(HAM_GRAPH_NODE_PIN_IN, "Test exec", ham::engine::graph_exec_type(ts));
		(void)test_in_exec_pin;

		const auto test_in_pin = test_node->new_pin(HAM_GRAPH_NODE_PIN_IN, "Test input", ts.get("i32"));
		(void)test_in_pin;

		const auto test_out_pin = test_node->new_pin(HAM_GRAPH_NODE_PIN_OUT, "Test output", ham::engine::graph_exec_type(ts));
		(void)test_out_pin;
	}

//	m_source_editor = new editor::source_editor(inner_widget);
//	m_source_editor->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
//	inner_lay->addWidget(m_source_editor);

	inner_lay->addWidget(m_graph_editor);
	inner_lay->addWidget(m_world_view);

	inner_widget->setLayout(inner_lay);

	if(!project_->parent()){
		project_->setParent(this);
	}

	set_central_widget(inner_widget);

//	const auto settings_img = QImage("://images/cog.png").scaledToWidth(32, Qt::SmoothTransformation);
//	const auto settings_pix = QPixmap::fromImage(settings_img);

//	const auto settings_btn = new QPushButton(this);
//	settings_btn->setFixedSize(32, 32);
//	settings_btn->setContentsMargins(0, 0, 0, 0);
//	settings_btn->setIcon(QIcon(settings_pix));
//	settings_btn->setIconSize(QSize(32, 32));

//	const auto header_btn_lay = new QHBoxLayout;
//	header_btn_lay->setContentsMargins(0, 0, 0, 0);
//	header_btn_lay->addWidget(settings_btn, 1, Qt::AlignLeft | Qt::AlignVCenter);

//	header()->set_gap_layout(window_header::gap::left, header_btn_lay);

	const auto graph_shown = settings.value(QString("%1/editor/show_graph").arg(proj_name_str), false).toBool();

	if(graph_shown){
		show_graph_editor();
	}
	else{
		hide_graph_editor();
	}

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
	delete m_engine;
}

void editor::main_window::show_graph_editor(bool do_show){
	QSettings settings;
	settings.setValue(QString("%1/editor/show_graph").arg(m_proj->name().toString()), do_show);

	if(do_show){
		m_graph_editor->show();
		m_world_view->show_overlay(false);
	}
	else{
		m_graph_editor->hide();
		m_world_view->show_overlay();
	}
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

void editor::main_window::keyReleaseEvent(QKeyEvent *event){
	switch(event->key()){
		case Qt::Key_G:{
			show_graph_editor(m_graph_editor->isHidden());
			event->accept();
			break;
		}

		default:{
			window::keyReleaseEvent(event);
			break;
		}
	}
}
