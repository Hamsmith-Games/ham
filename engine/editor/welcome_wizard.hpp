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

#ifndef HAM_ENGINE_EDITOR_WELCOME_WIZARD_HPP
#define HAM_ENGINE_EDITOR_WELCOME_WIZARD_HPP 1

#include <QMainWindow>
#include <QWizard>

#include "window.hpp"

namespace ham::engine::editor{
	class welcome_splash: public QWizardPage{
		Q_OBJECT

		public:
			welcome_splash(QWidget *parent = nullptr);
			~welcome_splash();
	};

	class welcome_wizard: public QWizard{
		Q_OBJECT

		public:
			welcome_wizard(QWidget *parent = nullptr);
			~welcome_wizard();
	};

	class welcome_window: public window{
		Q_OBJECT

		public:
			welcome_window(QWidget *parent = nullptr);
			~welcome_window();

		protected:
			void new_proj_pressed();
			void open_proj_pressed();

		private:
			QWidget *m_inner;
	};
}

#endif // !HAM_ENGINE_EDITOR_WELCOME_WIZARD_HPP
