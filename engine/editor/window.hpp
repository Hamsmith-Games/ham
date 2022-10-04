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
					resize_handle(Qt::Corner corner, class window *win, QWidget *parent = nullptr);

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
					header(class window *parent = nullptr);
					~header();

				protected:
					void mousePressEvent(QMouseEvent *event) override;
					void mouseReleaseEvent(QMouseEvent *event) override;

				private:
					QLabel *m_title_lbl;
			};

		public:
			window(QWidget *parent = nullptr);
			~window();

			void set_central_widget(QWidget *widget);
			QWidget *central_widget();

		signals:
			void maximized();
			void minimized();
			void normalized();

		protected:
			void changeEvent(QEvent *event) override;

		private:
			QLayout *m_lay;
			header *m_header;
			QWidget *m_inner;
	};
}

#endif // !HAM_ENGINE_EDITOR_WINDOW_HPP
