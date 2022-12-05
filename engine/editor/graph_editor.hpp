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

#ifndef HAM_ENGINE_GRAPH_EDITOR_HPP
#define HAM_ENGINE_GRAPH_EDITOR_HPP 1

#include "ham/typesys.h"
#include "ham/hash.h"
#include "ham/engine/graph.h"

#include <QVector2D>

#include <QJsonObject>

#include <QWidget>
#include <QMenu>
#include <QLabel>

#include <QGraphicsView>
#include <QGraphicsWidget>

Q_DECLARE_METATYPE(ham::type)
Q_DECLARE_METATYPE(ham::typeset_view)
Q_DECLARE_METATYPE(ham::const_typeset_view)

class QGraphicsScene;
class QGraphicsView;
class QGraphicsLinearLayout;

// predefs
namespace ham::engine::editor{
	class graph;
	class graph_node;
	class graph_node_pin;
}

namespace ham::engine::editor{
	static inline QColor string_color(ham::str8 str){
		if(str.is_empty()){
			return Qt::white;
		}

		const auto str_hash  = f64(ham::hash(str));
		const auto color_hue = 1.0 - (str_hash / f64(HAM_UPTR_MAX));

		return QColor::fromHsvF(color_hue, 1.f, 1.f);
	}

	ham_used
	static inline QColor string_color(const QString &str){
		const auto utf8_str = str.toUtf8();
		return string_color(str8(utf8_str.data(), utf8_str.size()-1));
	}

	class graph_node_pin: public QGraphicsWidget{
		Q_OBJECT

		Q_PROPERTY(QString name READ name WRITE set_name NOTIFY name_changed)
		Q_PROPERTY(ham::type pin_type READ pin_type WRITE set_pin_type NOTIFY pin_type_changed)
		Q_PROPERTY(QColor color READ color WRITE set_color NOTIFY color_changed)

		public:
			class pin_dot: public QGraphicsEllipseItem, public QGraphicsLayoutItem{
				public:
					pin_dot(qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent = nullptr)
						: QGraphicsEllipseItem(x, y, width, height, parent){}

					QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const override{
						Q_UNUSED(which);
						Q_UNUSED(constraint);
						return boundingRect().size();
					}

					void setGeometry(const QRectF &rect) override{
						setPos(rect.topLeft());
					}
			};

			class connection_curve: public QGraphicsItem{
				public:
					explicit connection_curve(const QPointF &begin_pos, bool is_output, const QColor &color = Qt::white, QGraphicsItem *parent = nullptr)
						: QGraphicsItem(parent), m_beg(begin_pos), m_end(begin_pos), m_color(color), m_is_out(is_output)
					{
						(void)m_is_out;
					}

					const QPointF &begin_pos() const noexcept{ return m_beg; }
					const QPointF &end_pos() const noexcept{ return m_end; }

					void set_end_pos(const QPointF &pos){
						m_end = pos;
						update();
					}

					QRectF boundingRect() const override;

					void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

				private:
					QPointF m_beg, m_end;
					QColor m_color;
					bool m_is_out;
			};

			explicit graph_node_pin(
				engine::graph_node_view node,
				ham_graph_node_pin_direction direction,
				const QString &name_,
				ham::type type_,
				QGraphicsItem *parent = nullptr
			);

			~graph_node_pin();

			QString name() const noexcept{ return m_name_lbl->text(); }

			const QColor &color() const noexcept{ return m_color; }

			ham_graph_node_pin_direction direction() const noexcept{ return m_pin.direction(); }

			bool is_input() const noexcept{ return direction() == HAM_GRAPH_NODE_PIN_IN; }
			bool is_output() const noexcept{ return direction() == HAM_GRAPH_NODE_PIN_OUT; }

			ham::type pin_type() const noexcept{ return m_pin.type(); }

			void set_name(const QString &new_name);

			void set_color(const QColor &new_color){
				if(!m_user_color || (m_color != new_color)){
					m_color = new_color;
					m_user_color = true;

					Q_EMIT color_changed(new_color);
				}
			}

			void set_pin_type(ham::type new_type){
				if(pin_type() != new_type){
					m_pin.set_type(new_type);
					Q_EMIT pin_type_changed(new_type);

					if(!m_user_color){
						m_color = string_color(new_type.name());
						Q_EMIT color_changed(m_color);
					}
				}
			}

			void reset_color(){
				if(m_user_color){
					m_color = string_color(m_pin.type().name());
					m_user_color = false;
					Q_EMIT color_changed(m_color);
				}
			}

		Q_SIGNALS:
			void name_changed(const QString&);
			void pin_type_changed(ham::type);
			void color_changed(const QColor&);

		protected:
			void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
			void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
			void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

		private:
			QLabel *m_name_lbl;
			engine::graph_node_pin_view m_pin;
			QColor m_color;
			pin_dot *m_dot;
			connection_curve *m_curve;

			bool m_user_color = false;
	};

	class graph_node: public QGraphicsWidget{
		Q_OBJECT

		Q_PROPERTY(QString name READ name WRITE set_name NOTIFY name_changed)
		Q_PROPERTY(QList<ham::engine::editor::graph_node_pin*> inputs READ inputs CONSTANT)
		Q_PROPERTY(QList<ham::engine::editor::graph_node_pin*> outputs READ outputs CONSTANT)

		public:
			graph_node(engine::graph *graph, const QString &name, QGraphicsItem *parent = nullptr);

			~graph_node(){
				for(auto pin : m_inputs){
					delete pin;
				}

				for(auto pin : m_outputs){
					delete pin;
				}
			}

			engine::const_graph_node_view node() const noexcept{ return m_node.ptr(); }

			QString name() const noexcept{ return m_name_lbl->text(); }

			const QList<graph_node_pin*> &inputs() const noexcept{ return m_inputs; }
			const QList<graph_node_pin*> &outputs() const noexcept{ return m_outputs; }

			void set_name(const QString &new_name){
				if(m_name_lbl->text() != new_name){
					const auto name_utf8 = new_name.toUtf8();
					m_name_lbl->setText(new_name);
					m_node.set_name(name_utf8.data());
					Q_EMIT name_changed(new_name);
				}
			}

			graph_node_pin *new_pin(ham_graph_node_pin_direction direction, const QString &name_, ham::type type);

			void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

			QJsonObject to_json() const;

			[[maybe_unused]]
			static graph_node *from_json(editor::graph *graph, const QJsonObject &obj);

		Q_SIGNALS:
			void name_changed(const QString&);

			void pin_added(ham::engine::editor::graph_node_pin*);
			void pin_removed(ham::engine::editor::graph_node_pin*);

		protected:
			QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

		private:
			QLabel *m_name_lbl;
			engine::graph_node_view m_node;
			QList<graph_node_pin*> m_inputs, m_outputs;
			QGraphicsLinearLayout *m_in_lay, *m_out_lay;
	};

	class graph_node_menu: public QMenu{
		public:
			explicit graph_node_menu(QWidget *parent = nullptr)
				: QMenu(parent){}

		private:
	};

	namespace detail{
		class graph_scene: public QGraphicsScene{
			Q_OBJECT

			public:
				explicit graph_scene(QString name_, QObject *parent = nullptr);

				void set_name(QAnyStringView new_name);

				qreal grid_size() const noexcept{ return m_grid_size; }

			protected:
				void drawBackground(QPainter *painter, const QRectF &rect);

			private:
				QString m_name;
				qreal m_grid_size;
		};
	}

	class graph: public QObject{
		Q_OBJECT

		Q_PROPERTY(QGraphicsScene* scene READ scene CONSTANT)

		Q_PROPERTY(QString name READ name WRITE set_name NOTIFY name_changed)
		Q_PROPERTY(QList<ham::engine::editor::graph_node*> nodes READ nodes CONSTANT)

		public:
			explicit graph(ham::engine::graph engine_graph, QObject *parent = nullptr);

			graph(QString name, ham::const_typeset_view ts, QObject *parent = nullptr);

			~graph();

			QGraphicsScene *scene() const noexcept{ return m_scene; }

			ham::const_typeset_view ts() const noexcept{ return m_graph.ts(); }

			const QString &name() const noexcept{ return m_name; }

			const QList<graph_node*> &nodes() const noexcept{ return m_nodes; }

			graph_node *new_node(QPointF pos, const QString &name);

			void set_name(const QString &new_name){
				if(m_name != new_name){
					const auto name_utf8 = new_name.toUtf8();
					m_graph.set_name(name_utf8.data());
					m_name = new_name;
					Q_EMIT name_changed(new_name);
				}
			}

			QJsonObject to_json() const;

			static graph *from_json(ham::const_typeset_view ts, const QJsonObject &obj);

		Q_SIGNALS:
			void name_changed(const QString&);

			void node_added(ham::engine::editor::graph_node*);
			void node_removed(ham::engine::editor::graph_node*);

		private:
			QGraphicsScene *m_scene;
			QString m_name;
			ham::engine::graph m_graph;
			QList<graph_node*> m_nodes;
	};

	class graph_menu: public QMenu{
		Q_OBJECT

		public:
			explicit graph_menu(QWidget *parent = nullptr);

			explicit graph_menu(editor::graph *graph, QWidget *parent = nullptr)
				: graph_menu(parent)
			{
				set_graph(graph);
			}

			editor::graph *graph() const noexcept{ return m_graph; }

			void set_graph(editor::graph *new_graph);

		Q_SIGNALS:
			void graph_changed(editor::graph*);

		private:
			editor::graph *m_graph;
	};

	class graph_editor: public QGraphicsView{
		Q_OBJECT

		Q_PROPERTY(ham::engine::editor::graph* graph READ graph CONSTANT)

		public:
			explicit graph_editor(editor::graph *graph = nullptr, QWidget *parent = nullptr);

			graph_editor(const QString &name, ham::const_typeset_view ts, QWidget *parent = nullptr)
				: graph_editor(new editor::graph(name, ts), parent){}

			~graph_editor();

			editor::graph *graph() const noexcept{ return m_graph; }

			graph_node *new_node(QPointF pos, const QString &name){
				return m_graph->new_node(pos, name);
			}

		protected:
			void mousePressEvent(QMouseEvent *event) override;
			void mouseReleaseEvent(QMouseEvent *event) override;
			void wheelEvent(QWheelEvent *event) override;

			void show_graph_menu(const QPoint &p);

		private:
			editor::graph *m_graph;
			qreal m_zoom_factor = 1.0;
	};
}

#endif // HAM_ENGINE_GRAPH_EDITOR_HPP
