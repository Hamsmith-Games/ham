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

#include "window.hpp"

#include <QGuiApplication>

#include <QMatrix4x4>

#include <QWindow>
#include <QMouseEvent>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>

#include <QSpacerItem>
#include <QPushButton>
#include <QLabel>

namespace editor = ham::engine::editor;

//
// Window resize handle
//

editor::window::resize_handle::resize_handle(Qt::Corner corner, editor::window *win, QWidget *parent)
	: QWidget(parent)
	, m_win(win)
{
	setFixedSize(32, 32);
	setContentsMargins(0, 0, 0, 0);

	const auto img_lbl = new QLabel(this);
	const QImage img("://images/resize-handle.png");
	img_lbl->setContentsMargins(0, 0, 0, 0);
	img_lbl->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	const auto img_pix = QPixmap::fromImage(img).scaledToWidth(32);

	switch(corner){
		case Qt::TopRightCorner:{
			QTransform trans;
			trans.scale(-1.0, 1.0);
			img_lbl->setPixmap(img_pix);
			m_edges = Qt::Edge::RightEdge | Qt::Edge::TopEdge;
			break;
		}

		case Qt::BottomLeftCorner:{
			QTransform trans;
			trans.scale( 1.0, -1.0);
			img_lbl->setPixmap(img_pix.transformed(trans));
			m_edges = Qt::Edge::LeftEdge | Qt::Edge::BottomEdge;
			break;
		}

		case Qt::BottomRightCorner:{
			QTransform trans;
			trans.scale(-1.0, -1.0);
			img_lbl->setPixmap(img_pix.transformed(trans));
			m_edges = Qt::Edge::RightEdge | Qt::Edge::BottomEdge;
			break;
		}

		case Qt::TopLeftCorner:
		default:
			img_lbl->setPixmap(img_pix);
			m_edges = Qt::Edge::LeftEdge | Qt::Edge::TopEdge;
			break;
	}
}

editor::window::resize_handle::~resize_handle(){}

void editor::window::resize_handle::enterEvent(QEnterEvent *event){
	QGuiApplication::setOverrideCursor(QCursor(Qt::SizeFDiagCursor));
}

void editor::window::resize_handle::leaveEvent(QEvent *event){
	QGuiApplication::restoreOverrideCursor();
}

void editor::window::resize_handle::mousePressEvent(QMouseEvent *event){
	const auto window_handle = m_win->windowHandle();
	if(window_handle){
		window_handle->startSystemResize(m_edges);
	}
}

//
// Window headers/title bars
//

editor::window::header::header(editor::window *parent)
	: QWidget(parent)
	, m_title_icn(new QLabel)
	, m_title_lbl(new QLabel)
{
	setContentsMargins(0, 0, 0, 0);
	setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);

	const QImage ham_img(":/images/ham.png");

	m_title_icn->setContentsMargins(0, 2, 0, 0);
	m_title_icn->setPixmap(QPixmap::fromImage(ham_img).scaledToWidth(16));

	connect(parent, &editor::window::windowTitleChanged, m_title_lbl, &QLabel::setText);

	QFont title_fnt = m_title_lbl->font();
	title_fnt.setPointSize(10);

	m_title_lbl->setFont(title_fnt);
	m_title_lbl->setAlignment(Qt::AlignCenter);
	m_title_lbl->setContentsMargins(0, 4, 0, 0);
	m_title_lbl->setTextFormat(Qt::TextFormat::MarkdownText);

	const auto title_lay = new QHBoxLayout;
	title_lay->setContentsMargins(0, 0, 0, 0);
	title_lay->addWidget(m_title_icn, 0, Qt::AlignVCenter);
	title_lay->addSpacing(15);
	title_lay->addWidget(m_title_lbl, 0, Qt::AlignVCenter);

	const auto resize_handle_tl = new editor::window::resize_handle(Qt::TopLeftCorner, parent, this);

	const auto min_btn = new QPushButton;
	min_btn->setContentsMargins(0, 0, 0, 0);

	const auto max_btn = new QPushButton;
	max_btn->setContentsMargins(0, 0, 0, 0);

	const auto close_btn = new QPushButton;
	close_btn->setContentsMargins(0, 0, 0, 0);

	connect(min_btn, &QPushButton::pressed, parent, &editor::window::showMinimized);

	connect(
		max_btn, &QPushButton::pressed,
		this, [max_btn, resize_handle_tl, parent]{
			if(parent->isMaximized()) parent->showNormal();
			else parent->showMaximized();
		}
	);

	connect(close_btn, &QPushButton::pressed, parent, &editor::window::close);

	connect(parent, &editor::window::maximized, resize_handle_tl, &QWidget::hide);
	connect(parent, &editor::window::normalized, resize_handle_tl, &QWidget::show);

	const QImage min_btn_img("://images/min-btn.png");
	const QImage max_btn_img("://images/maximize-btn.png");
	const QImage close_btn_img("://images/close-btn.png");

	const QIcon min_btn_icon(QPixmap::fromImage(min_btn_img).scaledToWidth(16, Qt::TransformationMode::FastTransformation));
	const QIcon max_btn_icon(QPixmap::fromImage(max_btn_img).scaledToWidth(16, Qt::TransformationMode::FastTransformation));
	const QIcon close_btn_icon(QPixmap::fromImage(close_btn_img).scaledToWidth(16, Qt::TransformationMode::FastTransformation));

	min_btn->setIcon(min_btn_icon);
	max_btn->setIcon(max_btn_icon);
	close_btn->setIcon(close_btn_icon);

	const auto win_btns_lay = new QHBoxLayout;
	win_btns_lay->setContentsMargins(0, 0, 4, 0);
	win_btns_lay->setAlignment(Qt::AlignRight);
	win_btns_lay->addWidget(min_btn, 0, Qt::AlignRight);
	win_btns_lay->addWidget(max_btn, 0, Qt::AlignRight);
	win_btns_lay->addWidget(close_btn, 0, Qt::AlignRight);

	const auto layout = new QGridLayout;

	layout->setContentsMargins(0, 0, 0, 0);
	layout->setColumnStretch(0, 1);
	layout->setColumnStretch(1, 0);
	layout->setColumnStretch(2, 1);
	layout->addWidget(resize_handle_tl, 0, 0, Qt::AlignLeft);
	layout->addLayout(title_lay, 0, 1, Qt::AlignHCenter);
	layout->addLayout(win_btns_lay, 0, 2, Qt::AlignRight);

	setLayout(layout);
}

editor::window::header::~header(){}

void editor::window::header::mousePressEvent(QMouseEvent *event){
	const auto window = qobject_cast<editor::window*>(parent());
	if(window){
		window->windowHandle()->startSystemMove();
		event->accept();
	}
}

void editor::window::header::mouseReleaseEvent(QMouseEvent *event){}

//
// Footer
//

editor::window::footer::footer(editor::window *parent)
	: QWidget(parent)
	, m_status_line(new QWidget)
{
	const auto status_lay = new QHBoxLayout;
	status_lay->setContentsMargins(0, 0, 0, 0);

	m_status_line->setContentsMargins(0, 0, 0, 0);
	m_status_line->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
	m_status_line->setLayout(status_lay);

	const auto resize_handle_br = new resize_handle(Qt::BottomRightCorner, parent, this);

	connect(parent, &editor::window::maximized, resize_handle_br, &QWidget::hide);
	connect(parent, &editor::window::normalized, resize_handle_br, &QWidget::show);

	const auto layout = new QHBoxLayout;
	layout->setContentsMargins(0, 0, 0, 0);

	layout->addWidget(m_status_line, 1);
	layout->addWidget(resize_handle_br, 0, Qt::AlignBottom | Qt::AlignRight);

	setLayout(layout);
}

editor::window::footer::~footer(){
}

void editor::window::footer::set_status_widget(QWidget *widget){
	const auto layout = m_status_line->layout();

	if(!layout->isEmpty()){
		const auto cur_item = layout->itemAt(0);
		layout->removeItem(cur_item);
	}

	layout->addWidget(widget);
}

//
// Windows
//

editor::window::window(QWidget *parent)
	: QWidget(parent)
	, m_lay(nullptr)
	, m_header(new class header(this))
	, m_inner(new QWidget)
	, m_footer(new class footer(this))
{
	setContentsMargins(0, 0, 0, 0);
	setWindowFlags(Qt::FramelessWindowHint);

	const auto inner_lay = new QVBoxLayout;
	inner_lay->setContentsMargins(0, 0, 0, 0);
	m_inner->setLayout(inner_lay);
	m_inner->setContentsMargins(0, 0, 0, 0);
	//m_inner->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

	const auto window_lay = new QVBoxLayout;
	window_lay->setContentsMargins(0, 0, 0, 0);
	window_lay->addWidget(m_header, 0, Qt::AlignTop);
	window_lay->addWidget(m_inner, 1);
	window_lay->addWidget(m_footer, 0, Qt::AlignBottom | Qt::AlignRight);

	setLayout(window_lay);

	m_lay = window_lay;
}

editor::window::~window(){}

void editor::window::set_central_widget(QWidget *widget){
	const auto layout = m_inner->layout();

	if(!layout->isEmpty()){
		const auto item = layout->takeAt(0);
		layout->removeItem(item);
		item->widget()->setParent(nullptr);
		item->widget()->hide();
	}

	layout->addWidget(widget);
	widget->show();
	widget->setFocus();
}

QWidget *editor::window::central_widget(){
	const auto layout = m_inner->layout();
	if(layout->isEmpty()) return nullptr;
	else return layout->takeAt(0)->widget();
}

void editor::window::changeEvent(QEvent *event){
	if(event->type() != QEvent::WindowStateChange) return;

	switch(windowState()){
		case Qt::WindowState::WindowMaximized:{
			Q_EMIT maximized();
			break;
		}

		case Qt::WindowState::WindowMinimized:{
			Q_EMIT minimized();
			break;
		}

		case Qt::WindowState::WindowNoState:{
			Q_EMIT normalized();
			break;
		}

		default: break;
	}
}
