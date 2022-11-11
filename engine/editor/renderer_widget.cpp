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

#include "renderer_widget.hpp"

#include <QDebug>
#include <QResizeEvent>

using namespace ham::typedefs;

namespace editor = ham::engine::editor;

editor::renderer_widget::renderer_widget(QWidget *parent)
	: QWidget(parent)
{
	setContentsMargins(0, 0, 0, 0);
}

void editor::renderer_widget::resizeEvent(QResizeEvent *ev){
	resize_renderer(ev->size().width(), ev->size().height());
}

bool editor::renderer_widget::initialize_renderer(const char *plugin_id, const char *obj_id, const ham_renderer_create_args *args){
	if(m_r){
		qWarning() << "Renderer already initialized";
		return false;
	}

	m_r = ham::renderer(plugin_id, obj_id, args);
	if(!m_r){
		qWarning() << "Error creating ham::renderer";
		return false;
	}

	m_ticker.reset();
	m_frame_data.common.current_frame = 0;
	return true;
}

void editor::renderer_widget::finalize_renderer(){
	if(!m_r) return;
	m_r = ham::renderer();
}

editor::renderer_widget::~renderer_widget(){
	finalize_renderer();
}

void editor::renderer_widget::resize_renderer(u32 w, u32 h){
	if(m_r && !m_r.resize(w, h)){
		qWarning() << "Error in ham_renderer_resize";
	}
}

void editor::renderer_widget::paint_renderer(){
	m_work.pop_all();

	const f64 dt = m_ticker.tick();

	Q_EMIT onFrame(dt, &m_frame_data.common);

	m_r.frame(dt, &m_frame_data);

	++m_frame_data.common.current_frame;
}
