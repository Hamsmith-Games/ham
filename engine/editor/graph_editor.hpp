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

#include <QVector2D>
#include <QWidget>
#include <QGraphicsView>
#include <QHash>

Q_DECLARE_METATYPE(ham::type)

class QGraphicsScene;
class QGraphicsView;

namespace ham::engine::editor{
	class node_pin: public QObject{
		Q_OBJECT

		Q_PROPERTY(QString name READ name WRITE set_name NOTIFY name_changed)
		Q_PROPERTY(ham::type type READ type WRITE set_type NOTIFY type_changed)

		public:
			explicit node_pin(QString name_, ham::type type_, QObject *parent = nullptr)
				: QObject(parent), m_name(std::move(name_)), m_type(type_){}

			const QString &name() const noexcept{ return m_name; }

			ham::type type() const noexcept{ return m_type; }

			void set_name(QAnyStringView new_name){
				if(m_name != new_name){
					m_name = new_name.toString();
					Q_EMIT name_changed(m_name);
				}
			}

			void set_type(ham::type new_type){
				if(m_type != new_type){
					m_type = new_type;
					Q_EMIT type_changed(m_type);
				}
			}

		Q_SIGNALS:
			void name_changed(const QString&);
			void type_changed(ham::type);

		private:
			QString m_name;
			ham::type m_type;
	};

	class node: public QObject{
		Q_OBJECT

		Q_PROPERTY(QString name READ name WRITE set_name NOTIFY name_changed)
		Q_PROPERTY(QList<ham::engine::editor::node_pin*> pins READ pins WRITE set_pins NOTIFY pins_changed)

		public:
			node(QString name, QList<node_pin*> pins, QObject *parent = nullptr)
				: QObject(parent), m_name(std::move(name)), m_pins(std::move(pins)){}

			explicit node(QString name, QObject *parent = nullptr)
				: node(std::move(name), {}, parent){}

			const QString &name() const noexcept{ return m_name; }
			const QList<node_pin*> &pins() const noexcept{ return m_pins; }

			void set_name(QAnyStringView new_name){
				if(new_name != m_name){
					m_name = new_name.toString();
					Q_EMIT name_changed(m_name);
				}
			}

			void set_pins(const QList<node_pin*> &new_pins){
				if(new_pins != m_pins){
					m_pins = new_pins;
					Q_EMIT pins_changed(m_pins);
				}
			}

			node_pin *create_pin(QString name_, ham::type type){
				const auto pin = new node_pin(std::move(name_), type, this);
				m_pins.emplaceBack(pin);
				Q_EMIT pins_changed(m_pins);
				return pin;
			}

			bool destroy_pin(node_pin *pin){
				const auto res = std::find(m_pins.cbegin(), m_pins.cend(), pin);
				if(res == m_pins.end()){
					return false;
				}

				m_pins.erase(res);
				Q_EMIT pins_changed(m_pins);
				return true;
			}

		Q_SIGNALS:
			void name_changed(const QString&);
			void pins_changed(const QList<ham::engine::editor::node_pin*>&);

		private:
			QString m_name;
			QList<node_pin*> m_pins;
	};

	class graph_node: public QObject{
		Q_OBJECT

		Q_PROPERTY(ham::engine::editor::node* node READ node WRITE set_node NOTIFY node_changed)
		Q_PROPERTY(QPointF pos READ pos WRITE set_pos NOTIFY pos_changed)

		public:
			graph_node(editor::node *node, QPointF pos, QObject *parent = nullptr)
				: QObject(parent), m_node(node), m_pos(pos)
			{
				node->setParent(this);
			}

			graph_node(QString name, QList<node_pin*> pins, QPointF pos, QObject *parent = nullptr)
				: graph_node(new editor::node(std::move(name), std::move(pins)), pos, parent){}

			graph_node(QString name, QPointF pos, QObject *parent = nullptr)
				: graph_node(new editor::node(std::move(name), {}, parent), pos, parent){}

			editor::node *node() const noexcept{ return m_node; }

			const QPointF &pos() const noexcept{ return m_pos; }

			void set_node(editor::node *new_node){
				if(m_node != new_node){
					m_node = new_node;
					Q_EMIT node_changed(new_node);
				}
			}

			void set_pos(QPointF new_pos){
				if(m_pos != new_pos){
					m_pos = new_pos;
					Q_EMIT pos_changed(new_pos);
				}
			}

			void translate(QVector2D amnt){
				if(amnt.lengthSquared() > 0){
					m_pos += QPointF(amnt.x(), amnt.y());
					Q_EMIT pos_changed(m_pos);
				}
			}

			node_pin *create_pin(QString name_, ham::type type){
				const auto pin = m_node->create_pin(std::move(name_), type);
				return pin;
			}

			bool destroy_pin(node_pin *pin){
				return m_node->destroy_pin(pin);
			}

		Q_SIGNALS:
			void node_changed(ham::engine::editor::node*);
			void pos_changed(QPointF);

		private:
			editor::node *m_node;
			QPointF m_pos;
	};

	class graph: public QObject{
		Q_OBJECT

		Q_PROPERTY(QList<ham::engine::editor::graph_node*> nodes READ nodes WRITE set_nodes NOTIFY nodes_changed)

		public:
			explicit graph(QObject *parent = nullptr)
				: QObject(parent){}

			const QList<graph_node*> &nodes() const noexcept{ return m_nodes; }

			void set_nodes(QList<graph_node*> new_nodes){
				if(new_nodes != m_nodes){
					m_nodes = std::move(new_nodes);
					for(auto node : m_nodes){
						node->setParent(this);
					}

					Q_EMIT nodes_changed(m_nodes);
				}
			}

			graph_node *create_node(QPointF pos, QString name, QList<node_pin*> pins){
				const auto inner = new node(name, std::move(pins), this);
				const auto ret   = new graph_node(inner, pos, this);

				m_nodes.append(ret);
				Q_EMIT nodes_changed(m_nodes);

				return ret;
			}

		Q_SIGNALS:
			void nodes_changed(const QList<ham::engine::editor::graph_node*>&);

		private:
			QList<graph_node*> m_nodes;
	};

	class graph_scene: public QObject{
		Q_OBJECT

		Q_PROPERTY(QGraphicsScene* graphics_scene READ graphics_scene CONSTANT)
		Q_PROPERTY(editor::graph* graph READ graph CONSTANT)

		public:
			explicit graph_scene(editor::graph *graph, QObject *parent = nullptr);

			explicit graph_scene(QObject *parent = nullptr)
				: graph_scene(new editor::graph, parent){}

			~graph_scene();

			QGraphicsScene *graphics_scene() noexcept{ return m_scene; }
			const QGraphicsScene *graphics_scene() const noexcept{ return m_scene; }

			editor::graph *graph() noexcept{ return m_graph; }
			const editor::graph *graph() const noexcept{ return m_graph; }

			graph_node *create_node(QPointF pos, QString name, QList<node_pin*> pins);

		private:
			QGraphicsScene *m_scene;
			editor::graph *m_graph;
			QHash<node_pin*, QGraphicsItem*> m_pin_items;
	};

	class graph_editor: public QWidget{
		Q_OBJECT

		Q_PROPERTY(ham::engine::editor::graph_scene* scene READ scene CONSTANT)
		Q_PROPERTY(QGraphicsView* graphics_view READ graphics_view CONSTANT)

		public:
			explicit graph_editor(graph_scene *scene = nullptr, QWidget *parent = nullptr);

			explicit graph_editor(QWidget *parent = nullptr)
				: graph_editor(new graph_scene, parent){}

			~graph_editor();

			graph_scene *scene() const noexcept{ return m_scene; }
			QGraphicsView *graphics_view() const noexcept{ return m_view; }

			graph_node *create_node(QPointF pos, QString name, QList<node_pin*> pins){
				return m_scene->create_node(pos, std::move(name), std::move(pins));
			}

		private:
			graph_scene *m_scene;
			QGraphicsView *m_view;
	};
}

#endif // HAM_ENGINE_GRAPH_EDITOR_HPP
