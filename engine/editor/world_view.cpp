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

#include "world_view.hpp"

#include "ham/engine/model.h"

#include <QSettings>
#include <QFile>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QPainter>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStackedLayout>
#include <QMenu>
#include <QLabel>
#include <QPushButton>

namespace editor = ham::engine::editor;

//
// World context menu
//

editor::world_context_menu::world_context_menu(editor::world_view *world, QWidget *parent)
	: QMenu(parent)
{
	const auto lbl = new QLabel("Hello", this);

	const auto vbox = new QVBoxLayout(this);

	vbox->addWidget(lbl);

	setLayout(vbox);

	//addAction(new QAction("Hello"));
}

editor::world_context_menu::~world_context_menu(){}

//
// Camera options widget
//

editor::camera_options_widget::camera_options_widget(ham::camera_view camera, QWidget *parent)
	: QWidget(parent)
	, m_cam(camera)
{}

editor::camera_options_widget::~camera_options_widget(){}

//
// World editor main view
//

editor::world_view::world_view(ham_engine *engine, ham_world *world, QWidget *parent)
	: QWidget(parent)
	, m_engine(engine)
	, m_world(world)
	, m_cam_dir(0.f)
{
	setContentsMargins(0, 0, 0, 0);
	setContextMenuPolicy(Qt::CustomContextMenu);

	m_r_widget = new editor::renderer_widget_gl(this);
	m_r_widget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

	m_overlay_widget = createOverlay();

	const auto view_stack = new QStackedLayout(this);

	view_stack->setContentsMargins(0, 0, 0, 0);
	view_stack->setStackingMode(QStackedLayout::StackAll);
	view_stack->addWidget(m_overlay_widget);
	view_stack->addWidget(m_r_widget);

	const auto lay = new QHBoxLayout(this);

	lay->setContentsMargins(0, 0, 0, 0);
	lay->addLayout(view_stack, 1);
	//lay->addWidget(m_r_widget, 1);

	m_r_widget->installEventFilter(this);

	setLayout(lay);

	QSettings settings;

	settings.beginGroup(QString("%1/editor/camera").arg(app_name()));

	const auto cam_pos = settings.value("pos", QVector3D(0.f, 0.f, 0.f)).value<QVector3D>();
	const auto cam_pyr = settings.value("pyr", QVector3D(0.f, 0.f, 0.f)).value<QVector3D>();

	m_cam.set_position({ cam_pos.x(), cam_pos.y(), cam_pos.z() });
	m_cam.set_pyr({ cam_pyr.x(), cam_pyr.y(), cam_pyr.z() });

	m_r_widget->do_work([this]{
		auto test_mdl_file = QFile("://models/xyz_test.glb");
		if(!test_mdl_file.open(QFile::ReadOnly)){
			ham::logapierror("Error in QFile(\"{}\")::open", "://models/xyz_test.glb");
			return;
		}

		const auto test_mdl_file_bytes = test_mdl_file.readAll();

		auto test_mdl = ham::model(test_mdl_file_bytes.size(), test_mdl_file_bytes.data());

		const auto quad_shape = ham_shape_unit_square();
		const ham_image *const default_img = ham_engine_get_default_tex_image(m_engine);

		test_mdl.fill_blank_images(default_img);

		m_test_group = ham::draw_group(m_r_widget->renderer(), test_mdl.num_shapes(), test_mdl.shapes(), test_mdl.images());
		m_test_group.set_num_instances(1);

		m_gizmo_group = ham::draw_group(m_r_widget->renderer(), 1, &quad_shape, &default_img);
		m_gizmo_group.set_num_instances(2);

		m_cam_light_group = ham::light_group(m_r_widget->renderer(), 1);

		ham::draw_group_instance_visit(m_gizmo_group, 0, [](ham_draw_group_instance_data *data){
			ham::transform trans;
			trans.translate({0.f, -0.5f, 0.f});
			trans.scale(ham::vec3(1.f));

			data->trans      = trans.matrix();
			data->normal_mat = ham_mat4_transpose(ham_mat4_inverse(data->trans));
		});

		ham::draw_group_instance_visit(m_gizmo_group, 1, [](ham_draw_group_instance_data *data){
			ham::transform trans;
			trans.translate({0.f, 0.5f, 0.f});
			trans.rotate(M_PI_2, {1.f, 0.f, 0.f});
			trans.scale(ham::vec3(5.f));

			data->trans      = trans.matrix();
			data->normal_mat = ham_mat4_transpose(ham_mat4_inverse(data->trans));
		});

		ham::light_group_instance_visit(m_cam_light_group, 0, [this](ham_light *light){
			light->pos = m_cam.position();
			light->effective_radius = 20.f;
			light->color = ham_make_vec3(1.f, 1.f, 1.f);
			light->intensity = 4.f;
		});
	});

	m_r_widget->frame_data().common.cam = m_cam.ptr();

	connect(m_r_widget, &renderer_widget::onFrame, this, [this](f64 dt, const ham_renderer_frame_data_common *data){
		if(!m_cam_held) return;

		const auto d_len = m_cam_dir.length();
		if(d_len < 0.0001) return;

		const auto dir = m_cam_dir / d_len;

		const auto dx = dir.x * m_cam.right();
		const auto dy = dir.y * m_cam.up();
		const auto dz = dir.z * m_cam.forward();

		const auto dp = dx + dy + dz;

		m_cam.translate(dt * ham_vec3_normalize(dp));

		ham::light_group_instance_visit(m_cam_light_group, 0, [this](ham_light *light){
			light->pos = m_cam.position();
		});
	});

	connect(this, &QWidget::customContextMenuRequested, this, &world_view::show_context_menu);
}

editor::world_view::~world_view(){}

void editor::world_view::show_context_menu(const QPoint &pos){
	editor::world_context_menu ctx_menu(this);
	ctx_menu.exec(mapToGlobal(pos));
}

QWidget *editor::world_view::createOverlay(){
	const auto lay = new QGridLayout;

	QImage cog_img("://images/settings.png");
	QImage cam_img("://images/camera.png");

	const auto cog_pix = QPixmap::fromImage(cog_img);
	const auto cam_pix = QPixmap::fromImage(cam_img);

	const auto cog_btn = new QPushButton;
	const auto cam_btn = new QPushButton;

	QPixmap cog_icon_pix(cog_pix.size());
	QPixmap cam_icon_pix(cam_pix.size());

	cog_icon_pix.fill(Qt::transparent);
	cam_icon_pix.fill(Qt::transparent);

	{
		QPainter icon_painter;
		icon_painter.begin(&cam_icon_pix);
		icon_painter.setOpacity(0.66f);
		icon_painter.drawPixmap(0, 0, cam_pix);
		icon_painter.end();

		icon_painter.begin(&cog_icon_pix);
		icon_painter.setOpacity(0.66f);
		icon_painter.drawPixmap(0, 0, cog_pix);
		icon_painter.end();
	}

	cog_btn->setIcon(cog_icon_pix);
	cog_btn->setIconSize(QSize{48, 48});

	cam_btn->setIcon(cam_icon_pix);
	cam_btn->setIconSize(QSize{48, 48});

	lay->setContentsMargins(0, 0, 0, 0);
	lay->addWidget(cog_btn, 0, 0, Qt::AlignLeft | Qt::AlignTop);
	lay->addWidget(cam_btn, 0, 1, Qt::AlignRight | Qt::AlignTop);

	const auto ret = new QWidget;
	ret->setContentsMargins(10, 10, 10, 10);
	ret->setLayout(lay);

	return ret;
}

bool editor::world_view::eventFilter(QObject *watched, QEvent *ev){
	if(watched == m_r_widget){
		switch(ev->type()){
			case QEvent::MouseButtonPress:{
				mousePressEvent(static_cast<QMouseEvent*>(ev));
				return true;
			}

			case QEvent::MouseButtonRelease:{
				mouseReleaseEvent(static_cast<QMouseEvent*>(ev));
				return true;
			}

			case QEvent::MouseMove:{
				mouseMoveEvent(static_cast<QMouseEvent*>(ev));
				return true;
			}

			default: break;
		}
	}

	return QWidget::eventFilter(watched, ev);
}

void editor::world_view::mouseMoveEvent(QMouseEvent *ev){
	if(m_cam_held){
		const QPointF cur_pos = ev->pos();

		QPointF dpos = cur_pos - m_cam_last_pos;
		dpos = QPointF(dpos.x() / width(), dpos.y() / height());

		m_cam_last_pos = cur_pos;
		m_cam.rotate(2.0 * M_PI, ham::vec3(dpos.y(), dpos.x(), 0.f));
	}
	else{
		QWidget::mouseMoveEvent(ev);
	}
}

void editor::world_view::mousePressEvent(QMouseEvent *ev){
	if(!m_cam_held && ev->button() == m_cam_btn){
		ev->accept();
		m_cam_held = true;
		m_cam_last_pos = ev->pos();
		m_cam_dir = {0.f, 0.f, 0.f};
		grabKeyboard();
		//grabMouse();
	}
	else{
		QWidget::mousePressEvent(ev);
	}
}

void editor::world_view::mouseReleaseEvent(QMouseEvent *ev){
	if(ev->button() == m_cam_btn){
		ev->accept();
		m_cam_held = false;
		m_cam_dir = {0.f, 0.f, 0.f};
		releaseKeyboard();
		//releaseMouse();

		QSettings settings;

		settings.beginGroup(QString("%1/editor/camera").arg(app_name()));

		const auto cam_pos = m_cam.position();
		const auto cam_pyr = m_cam.pyr();

		settings.setValue("pos", QVector3D(cam_pos.x(), cam_pos.y(), cam_pos.z()));
		settings.setValue("pyr", QVector3D(cam_pyr.x(), cam_pyr.y(), cam_pyr.z()));

		return;
	}

	switch(ev->button()){
		case Qt::MouseButton::RightButton:{
			ev->accept();
			show_context_menu(ev->pos());
			return;
		}

		default:{
			QWidget::mouseReleaseEvent(ev);
			return;
		}
	}
}

void editor::world_view::keyPressEvent(QKeyEvent *ev){
	if(!m_cam_held){
		QWidget::keyPressEvent(ev);
		return;
	}

	switch(ev->key()){
		case Qt::Key_W:
			m_cam_dir.z() += 1.f;
			break;

		case Qt::Key_S:
			m_cam_dir.z() -= 1.f;
			break;

		case Qt::Key_A:
			m_cam_dir.x() -= 1.f;
			break;

		case Qt::Key_D:
			m_cam_dir.x() += 1.f;
			break;

		case Qt::Key_Control:
			m_cam_dir.y() -= 1.f;
			break;

		case Qt::Key_Space:
			m_cam_dir.y() += 1.f;
			break;

		default:
			break;
	}
}

void editor::world_view::keyReleaseEvent(QKeyEvent *ev){
	if(!m_cam_held){
		QWidget::keyPressEvent(ev);
		return;
	}

	switch(ev->key()){
		case Qt::Key_W:
			m_cam_dir.z() -= 1.f;
			break;

		case Qt::Key_S:
			m_cam_dir.z() += 1.f;
			break;

		case Qt::Key_A:
			m_cam_dir.x() += 1.f;
			break;

		case Qt::Key_D:
			m_cam_dir.x() -= 1.f;
			break;

		case Qt::Key_Control:
			m_cam_dir.y() += 1.f;
			break;

		case Qt::Key_Space:
			m_cam_dir.y() -= 1.f;
			break;

		default:
			break;
	}
}

void editor::world_view::resizeEvent(QResizeEvent *ev){
	QWidget::resizeEvent(ev);

	//settings.setValue(QString("%1/editor/size").arg(app_name()), ev->size());

	QSettings settings;
	settings.beginGroup(QString("%1/editor/camera").arg(app_name()));

	const auto new_size = ev->size();

	// a == aspect
	// fov_x = 2 * atan(tan(fov_y * 0.5) * aspect)
	// fov_y = (2 * tan(fov_x * 0.5)) / a

	const qreal aspect = f64(new_size.width())/f64(new_size.height());

	const qreal fov_x = settings.value("fov", qreal(M_PI_2 - 0.001)).toReal();
	const qreal fov_y = (2.0 * qTan(fov_x * 0.5)) / aspect;

	const auto cam_zlimits = settings.value("zlimits", QVector2D(0.001f, 1000.f)).value<QVector2D>();

	m_cam.set_perspective_rev(aspect, fov_y, cam_zlimits.x(), cam_zlimits.y());
}
