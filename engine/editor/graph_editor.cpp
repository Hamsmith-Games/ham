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

#include "ham/log.h"

#include "graph_editor.hpp"

#include <QApplication>

#include <QHBoxLayout>

#include <QLabel>
#include <QOpenGLWidget>

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsItemGroup>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsLinearLayout>
#include <QGraphicsProxyWidget>

using namespace ham::typedefs;

namespace editor = ham::engine::editor;

constexpr static qreal node_title_size = 14.f;
constexpr static qreal node_title_margin = 10.f;
constexpr static qreal pin_radius = 7.5f;
constexpr static qreal pin_spacing = 10.f;

//
// Graph node pins
//

editor::graph_node_pin::graph_node_pin(
	engine::graph_node_view node,
	ham_graph_node_pin_direction direction,
	const QString &name_,
	ham::type type_,
	QGraphicsItem *parent
)
	: QGraphicsWidget(parent)
	, m_name_lbl(new QLabel(name_))
	, m_pin(node.create_pin(direction, "NULL", type_))
	, m_color(string_color(type_.name()))
	, m_dot(new pin_dot(0, 0, pin_radius * 2.f, pin_radius * 2.f, this))
	, m_curve(nullptr)
{
	const auto name_utf8 = name_.toUtf8();
	m_pin.set_name(name_utf8.data());

	QColor dot_color = m_color;
	dot_color.setAlpha(128);

	m_dot->setBrush(dot_color);

	connect(this, &graph_node_pin::color_changed, this, [this](const QColor &color){
		QColor dot_color = m_color;
		dot_color.setAlpha(128);

		m_dot->setBrush(dot_color);
	});

	auto fnt = QApplication::font();
	fnt.setPointSizeF(10.f);

	m_name_lbl->setFont(fnt);
	m_name_lbl->setStyleSheet("background: transparent; border: none");
	m_name_lbl->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	const auto name_proxy = new QGraphicsProxyWidget(this);
	name_proxy->setWidget(m_name_lbl);

	auto lay = new QGraphicsLinearLayout(Qt::Horizontal, this);
	lay->setContentsMargins(0.f, pin_spacing, 0.f, pin_spacing);

	if(is_input()){
		lay->addItem(m_dot);
		lay->addItem(name_proxy);
	}
	else{
		lay->addItem(name_proxy);
		lay->addItem(m_dot);
	}

	//lay->setItemSpacing(0, pin_spacing);
	lay->setAlignment(name_proxy, Qt::AlignVCenter);
	lay->setStretchFactor(name_proxy, 0);

	setContentsMargins(0.f, 0.f, 0.f, 0.f);
	setLayout(lay);
}

editor::graph_node_pin::~graph_node_pin(){}

void editor::graph_node_pin::set_name(const QString &new_name){
	if(m_name_lbl->text() != new_name){
		const auto name_utf8 = new_name.toUtf8();
		m_pin.set_name(name_utf8.data());

		m_name_lbl->setText(new_name);

		Q_EMIT name_changed(new_name);
	}
}

void editor::graph_node_pin::mousePressEvent(QGraphicsSceneMouseEvent *event){
	const auto press_pos = event->pos().toPoint();
	const auto mapped_pos = mapToItem(m_dot, press_pos);

//	const auto dot_rect = m_dot->rect();
//	const auto dot_tl = dot_rect.topLeft();
//	const auto dot_br = dot_rect.bottomRight();

//	ham::loginfo("GRAPH TEST", "Dot rect:       ({}, {}) ({}, {})", dot_tl.x(), dot_tl.y(), dot_br.x(), dot_br.y());
//	ham::loginfo("GRAPH TEST", "Press position: ({}, {})", mapped_pos.x(), mapped_pos.y());

	if(m_dot->shape().contains(mapped_pos)){
		ham::loginfo("GRAPH TEST", "Dot pressed");
		if(m_curve){
			delete m_curve;
		}

		const auto conn_pos = mapToScene(QPointF{ pin_radius, pin_radius });

		m_curve = new graph_node_pin::connection_curve(conn_pos, this->is_output(), m_color, this);
	}
	else{
		QGraphicsWidget::mousePressEvent(event);
	}
}

void editor::graph_node_pin::mouseMoveEvent(QGraphicsSceneMouseEvent *event){
	if(!m_curve){
		QGraphicsItem::mouseMoveEvent(event);
		return;
	}

	const auto cursor_pos = mapToScene(event->pos());

	m_curve->set_end_pos(cursor_pos);
}

void editor::graph_node_pin::mouseReleaseEvent(QGraphicsSceneMouseEvent *event){
	if(!m_curve){
		QGraphicsItem::mouseReleaseEvent(event);
		return;
	}

	delete m_curve;
	m_curve = nullptr;

	// TODO: get release position and check for node
}

//
// Graph nodes
//

editor::graph_node::graph_node(engine::graph *graph, const QString &name, QGraphicsItem *parent)
	: QGraphicsWidget(parent)
	, m_name_lbl(new QLabel(name))
	, m_node(graph->create_node("NULL"))
	, m_in_lay(new QGraphicsLinearLayout(Qt::Vertical))
	, m_out_lay(new QGraphicsLinearLayout(Qt::Vertical))
{
	const auto name_utf8 = name.toUtf8();
	m_node.set_name(name_utf8.data());

	//this->setAutoFillBackground(true);

	auto name_fnt = QApplication::font();
	name_fnt.setPointSizeF(node_title_size);

	m_name_lbl->setStyleSheet("background: transparent; border: none");
	m_name_lbl->setFont(name_fnt);
	m_name_lbl->setContentsMargins(node_title_margin + (pin_radius * 2.f), node_title_margin + pin_radius, node_title_margin, node_title_margin);

	const auto name_proxy = new QGraphicsProxyWidget(this);
	name_proxy->setWidget(m_name_lbl);

	// top-level layout incl name
	const auto top_lay = new QGraphicsLinearLayout(Qt::Vertical, this);
	top_lay->setContentsMargins(0.f, 0.f, 0.f, 0.f);
	top_lay->addItem(name_proxy);
	top_lay->setInstantInvalidatePropagation(true);

	// inner pin layouts
	const auto inner_lay = new QGraphicsLinearLayout(Qt::Horizontal, top_lay);
	inner_lay->setContentsMargins(0.f, 0.f, 0.f, 0.f);
	inner_lay->addItem(m_in_lay);
	inner_lay->setItemSpacing(0, 50.f);
	inner_lay->addItem(m_out_lay);
	inner_lay->setInstantInvalidatePropagation(true);

	top_lay->addItem(inner_lay);

	setCacheMode(QGraphicsItem::DeviceCoordinateCache);
	setContentsMargins(0.f, 0.f, 0.f, 0.f);
	setLayout(top_lay);
}

editor::graph_node_pin *editor::graph_node::new_pin(ham_graph_node_pin_direction direction, const QString &name_, ham::type type){
	const auto ret = new graph_node_pin(m_node, direction, name_, type, this);

	QGraphicsLinearLayout *pin_lay = nullptr;
	QList<editor::graph_node_pin*> *pin_list = nullptr;

	if(ret->is_input()){
		pin_lay = m_in_lay;
		pin_list = &m_inputs;
	}
	else{
		pin_lay = m_out_lay;
		pin_list = &m_outputs;
	}

	connect(ret, &QObject::destroyed, this, [this, pin_lay, pin_list](QObject *obj){
		const auto pin = qobject_cast<graph_node_pin*>(obj);
		pin_lay->removeItem(pin);
		pin_list->removeOne(pin);

		pin_lay->invalidate();

		Q_EMIT pin_removed(pin);
	});

	pin_lay->addItem(ret);
	pin_list->append(ret);

	pin_lay->invalidate();

	Q_EMIT pin_added(ret);

	return ret;
}

void editor::graph_node::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
	QRectF rect = boundingRect();
	rect.setWidth(rect.width() - pin_radius);
	rect.setLeft(pin_radius);

	painter->setBrush(QColor(105, 105, 105));
	painter->drawRoundedRect(rect, 10.f, 10.f);

	QGraphicsWidget::paint(painter, option, widget);
}

//
// Graphs
//

editor::graph::graph(QString name, QObject *parent)
	: QObject(parent)
	, m_scene(new QGraphicsScene(this))
	, m_name(std::move(name))
	, m_graph("NULL")
{
	const auto name_utf8 = m_name.toUtf8();
	m_graph.set_name(name_utf8.data());
}

editor::graph::~graph(){
	for(auto node : m_nodes){
		delete node;
	}
}

editor::graph_node *editor::graph::new_node(QPointF pos, const QString &name){
	const auto node = new editor::graph_node(&m_graph, name);
	node->setParent(this);

	connect(node, &QObject::destroyed, this, [this](QObject *obj){
		const auto node = qobject_cast<editor::graph_node*>(obj);
		m_scene->removeItem(node);
		m_nodes.removeOne(node);
		Q_EMIT node_removed(node);
	});

	m_nodes.append(node);

	node->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);

	m_scene->addItem(node);

	Q_EMIT node_added(node);

	return node;
}

//
// Graph view/editor
//

editor::graph_editor::graph_editor(editor::graph *graph, QWidget *parent)
	: QWidget(parent)
	, m_graph(graph)
	, m_view(new QGraphicsView(graph->scene(), this))
{
	Q_ASSERT(graph != nullptr);

	m_graph->setParent(this);

	QPalette pal = palette();
	pal.setColor(QPalette::All, QPalette::Base, QColor(0, 0, 0, 0));

	setPalette(pal);
	setContentsMargins(0, 0, 0, 0);
	setAttribute(Qt::WA_TranslucentBackground);

	pal.setColor(QPalette::All, QPalette::Base, QColor(105, 105, 105, 64));

	m_view->setPalette(pal);
	//m_view->setBackgroundBrush(QColor(105, 105, 105, 64));
	//m_view->setViewport(new QOpenGLWidget);
	//m_view->setAttribute(Qt::WA_AlwaysStackOnTop);
	m_view->setAttribute(Qt::WA_TranslucentBackground);

	const auto lay = new QHBoxLayout(this);

	lay->setContentsMargins(0, 0, 0, 0);
	lay->addWidget(m_view);

	setLayout(lay);

	m_view->show();
}

editor::graph_editor::~graph_editor(){}
