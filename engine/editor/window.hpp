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

#ifndef HAM_ENGINE_EDITOR_WINDOW_HPP
#define HAM_ENGINE_EDITOR_WINDOW_HPP 1

#include <QWidget>

class QLabel;

namespace ham::engine::editor{
	class window;

	class window: public QWidget{
		Q_OBJECT

		private:
			class resize_handle: public QWidget{
				public:
					explicit resize_handle(Qt::Corner corner, class window *win, QWidget *parent = nullptr);

					explicit resize_handle(class window *win, QWidget *parent = nullptr)
						: resize_handle(Qt::TopLeftCorner, win){}

					~resize_handle();

				protected:
					void enterEvent(QEnterEvent *event) override;
					void leaveEvent(QEvent *event) override;

					void mousePressEvent(QMouseEvent *event) override;

				private:
					class window *m_win;
					Qt::Edges m_edges;
			};

			class header: public QWidget{
				public:
					explicit header(class window *parent);
					~header();

				protected:
					void mousePressEvent(QMouseEvent *event) override;
					void mouseReleaseEvent(QMouseEvent *event) override;

				private:
					QLabel *m_title_icn;
					QLabel *m_title_lbl;
			};

			class footer: public QWidget{
				public:
					explicit footer(class window *parent);
					~footer();

					void set_status_widget(QWidget *widget);

				private:
					QWidget *m_status_line;
			};

		public:
			explicit window(QWidget *parent = nullptr);
			~window();

			void set_central_widget(QWidget *widget);
			QWidget *central_widget();

			class header *header() noexcept{ return m_header; }
			class footer *footer() noexcept{ return m_footer; }

		Q_SIGNALS:
			void maximized();
			void minimized();
			void normalized();

		protected:
			void changeEvent(QEvent *event) override;

		private:
			QLayout *m_lay;
			class header *m_header;
			QWidget *m_inner;
			class footer *m_footer;
	};
}

#endif // !HAM_ENGINE_EDITOR_WINDOW_HPP
