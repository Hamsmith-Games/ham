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

#ifndef HAM_ENGINE_EDITOR_PHYSICS_BACKEND_HPP
#define HAM_ENGINE_EDITOR_PHYSICS_BACKEND_HPP 1

#include "ham/physics.h"
#include "ham/plugin.h"

#include <QObject>

namespace ham::engine::editor{
	class physics_backend: public QObject{
		Q_OBJECT

		public:
			explicit physics_backend(QObject *parent = nullptr);

			~physics_backend();

			physics_view get() const noexcept{ return m_handle.get(); }

		private:
			ham_plugin *m_plug;
			ham_dso_handle m_dso;
			unique_handle<ham_physics*, ham_physics_destroy> m_handle;
	};
}

#endif // !ENGINE_EDITOR_PHYSICS_BACKEND_HPP
