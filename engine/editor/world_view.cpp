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

#include "world_view.hpp"

#include "ham/engine/model.h"

#include <QSettings>
#include <QFile>
#include <QMouseEvent>
#include <QKeyEvent>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMenu>
#include <QLabel>

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

	const auto lay = new QHBoxLayout(this);

	lay->setContentsMargins(0, 0, 0, 0);
	lay->addWidget(m_r_widget, 1);

	m_r_widget->installEventFilter(this);

	setLayout(lay);

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

			data->trans = trans.matrix();
		});

		ham::draw_group_instance_visit(m_gizmo_group, 1, [](ham_draw_group_instance_data *data){
			ham::transform trans;
			trans.translate({0.f, 0.5f, 0.f});
			trans.rotate(M_PI_2, {1.f, 0.f, 0.f});
			trans.scale(ham::vec3(5.f));

			data->trans = trans.matrix();
		});

		ham::light_group_instance_visit(m_cam_light_group, 0, [](ham_light *light){
			light->pos = ham_make_vec3(0.f, 0.f, 0.f);
			light->effective_radius = 20.f;
			light->color = ham_make_vec3(1.f, 1.f, 1.f);
			light->intensity = 1.f;
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

	QSettings settings;
	settings.setValue("engine/size", ev->size());

	const auto new_size = ev->size();

	m_cam.set_perspective_rev(f64(new_size.width())/f64(new_size.height()), M_PI_2 - 0.001f, 0.001f, 100.f);
}
