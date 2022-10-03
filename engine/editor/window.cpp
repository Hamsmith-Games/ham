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
	, m_title_lbl(new QLabel)
{
	setContentsMargins(0, 0, 0, 0);
	setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);

	connect(parent, &editor::window::windowTitleChanged, m_title_lbl, &QLabel::setText);

	const auto resize_handle_tl = new editor::window::resize_handle(Qt::TopLeftCorner, parent, this);

	const auto min_btn = new QPushButton;
	const auto max_btn = new QPushButton;
	const auto close_btn = new QPushButton;

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

	const QIcon min_btn_icon(QPixmap::fromImage(min_btn_img));
	const QIcon max_btn_icon(QPixmap::fromImage(max_btn_img));
	const QIcon close_btn_icon(QPixmap::fromImage(close_btn_img));

	min_btn->setIcon(min_btn_icon);
	max_btn->setIcon(max_btn_icon);
	close_btn->setIcon(close_btn_icon);

	const auto win_btns_lay = new QHBoxLayout;
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
	layout->addWidget(m_title_lbl, 0, 1, Qt::AlignHCenter);
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
// Windows
//

editor::window::window(QWidget *parent)
	: QWidget(parent)
	, m_lay(nullptr)
	, m_header(new header(this))
	, m_inner(new QWidget)
{
	setContentsMargins(0, 0, 0, 0);
	setWindowFlags(Qt::FramelessWindowHint);

	const auto resize_handle_br = new resize_handle(Qt::BottomRightCorner, this);

	const auto inner_lay = new QVBoxLayout;
	m_inner->setLayout(inner_lay);

	const auto window_lay = new QVBoxLayout;

	m_inner->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

	window_lay->setContentsMargins(0, 0, 0, 0);

	window_lay->addWidget(m_header, 0, Qt::AlignTop);
	window_lay->addWidget(m_inner, 1);
	window_lay->addWidget(resize_handle_br, 0, Qt::AlignBottom | Qt::AlignRight);

	connect(this, &editor::window::maximized, resize_handle_br, &QWidget::hide);
	connect(this, &editor::window::normalized, resize_handle_br, &QWidget::show);

	setLayout(window_lay);

	m_lay = window_lay;
}

editor::window::~window(){}

void editor::window::set_central_widget(QWidget *widget){
	const auto layout = m_inner->layout();

	const auto &inner_children = m_inner->children();
	if(!inner_children.empty()){
		const auto child = m_inner->childAt(0, 0);
		layout->removeWidget(child);
	}

	layout->addWidget(widget);
}

void editor::window::changeEvent(QEvent *event){
	if(event->type() != QEvent::WindowStateChange) return;

	switch(windowState()){
		case Qt::WindowState::WindowMaximized:{
			emit maximized();
			break;
		}

		case Qt::WindowState::WindowMinimized:{
			emit minimized();
			break;
		}

		case Qt::WindowState::WindowNoState:{
			emit normalized();
			break;
		}

		default: break;
	}
}
