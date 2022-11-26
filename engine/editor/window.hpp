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

#ifndef HAM_ENGINE_EDITOR_WINDOW_HPP
#define HAM_ENGINE_EDITOR_WINDOW_HPP 1

#include <QWidget>

class QLabel;

namespace ham::engine::editor{
	class window;

	class window_resize_handle: public QWidget{
		Q_OBJECT

		public:
			~window_resize_handle();

		protected:
			void enterEvent(QEnterEvent *event) override;
			void leaveEvent(QEvent *event) override;

			void mousePressEvent(QMouseEvent *event) override;

		private:
			explicit window_resize_handle(Qt::Corner corner, class window *win, QWidget *parent = nullptr);

			explicit window_resize_handle(class window *win, QWidget *parent = nullptr)
				: window_resize_handle(Qt::TopLeftCorner, win){}

			class window *m_win;
			Qt::Edges m_edges;

			friend class window;
			friend class window_header;
			friend class window_footer;
	};

	class window_header: public QWidget{
		Q_OBJECT

		public:
			enum class gap{
				left, right, bottom
			};

			~window_header();

			void set_gap_widget(enum gap gap, QWidget *w);
			void set_gap_layout(enum gap gap, QLayout *l);

		protected:
			void mousePressEvent(QMouseEvent *event) override;
			void mouseReleaseEvent(QMouseEvent *event) override;

		private:
			explicit window_header(class window *parent);

			QLabel *m_title_icn;
			QLabel *m_title_lbl;

			friend class window;
	};

	class window_footer: public QWidget{
		Q_OBJECT

		public:
			~window_footer();

			void set_status_widget(QWidget *widget);

		private:
			explicit window_footer(class window *parent);

			QWidget *m_status_line;

			friend class window;
	};

	class window: public QWidget{
		Q_OBJECT

		public:
			explicit window(QWidget *parent = nullptr);
			~window();

			void set_central_widget(QWidget *widget);
			QWidget *central_widget();

			class window_header *header() noexcept{ return m_header; }
			class window_footer *footer() noexcept{ return m_footer; }

		Q_SIGNALS:
			void maximized();
			void minimized();
			void normalized();

		protected:
			void changeEvent(QEvent *event) override;

		private:
			QLayout *m_lay;
			window_header *m_header;
			QWidget *m_inner;
			window_footer *m_footer;
	};
}

#endif // !HAM_ENGINE_EDITOR_WINDOW_HPP
