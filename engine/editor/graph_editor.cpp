#include "graph_editor.hpp"

#include <QHBoxLayout>

#include <QGraphicsRectItem>
#include <QGraphicsItemGroup>
#include <QGraphicsScene>
#include <QGraphicsView>

using namespace ham::typedefs;

namespace editor = ham::engine::editor;

//
// Graph "scene"
//

editor::graph_scene::graph_scene(editor::graph *graph, QObject *parent)
	: QObject(parent)
	, m_scene(new QGraphicsScene(this))
	, m_graph(graph)
{
	Q_ASSERT(graph != nullptr);

	graph->setParent(this);

	//m_scene->setBackgroundBrush(QBrush(QColor(0, 0, 0, 0)));
}

editor::graph_scene::~graph_scene(){

}

static inline QRectF calculate_node_rect(const QGraphicsScene *scene, const editor::node *node){
	const auto fnt_metrics = QFontMetricsF(scene->font());
	const auto text_rect = fnt_metrics.boundingRect(node->name());

	const auto num_pins = node->pins().size();

	return {
		QPointF(0.f, 0.f),
		QSizeF{text_rect.width() + 20.f, (num_pins * 10.f) + (text_rect.height() + 20.f)}
	};
}

editor::graph_node *editor::graph_scene::create_node(QPointF pos, QString name, QList<node_pin*> pins){
	const auto node = new editor::node(std::move(name), std::move(pins), this);

	const auto node_item = new QGraphicsItemGroup;
	node_item->setPos(pos);
	node_item->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);

	m_scene->addItem(node_item);

	const auto node_rect_item = m_scene->addRect(calculate_node_rect(m_scene, node));
	node_rect_item->setBrush(QColor(128, 128, 128, 255));
	node_rect_item->setParentItem(node_item);

	const auto node_name_item = m_scene->addText(node->name());
	node_name_item->setPos(QPointF(10.f, 0.f));
	node_name_item->setParentItem(node_rect_item);

	node_item->addToGroup(node_rect_item);
	node_item->addToGroup(node_name_item);

	auto pin_fnt = m_scene->font();
	pin_fnt.setPointSizeF(10.f);

	for(auto pin : node->pins()){
		const auto pin_name_item = m_scene->addText(pin->name());
		node_item->addToGroup(pin_name_item);
	}

	const auto ret = new editor::graph_node(node, pos, m_graph);

	connect(ret->node(), &editor::node::pins_changed, this, [this, ret, node_rect_item](const QList<node_pin*> &pins){
		node_rect_item->setRect(calculate_node_rect(m_scene, ret->node()));


	});

	connect(ret, &editor::graph_node::pos_changed, this, [node_item](QPointF pos){
		node_item->setPos(pos);
	});

	return ret;
}

//
// Graph view/editor
//

editor::graph_editor::graph_editor(editor::graph_scene *scene, QWidget *parent)
	: QWidget(parent)
	, m_scene(scene)
	, m_view(new QGraphicsView(scene->graphics_scene(), this))
{
	Q_ASSERT(scene != nullptr);
	m_scene->setParent(this);

	QPalette pal = palette();
	pal.setColor(QPalette::All, QPalette::Base, QColor(0, 0, 0, 0));

	setPalette(pal);
	setContentsMargins(0, 0, 0, 0);
	setAttribute(Qt::WA_TranslucentBackground);

	pal.setColor(QPalette::All, QPalette::Base, QColor(105, 105, 105, 64));

	m_view->setPalette(pal);
	//m_view->setBackgroundBrush(QColor(105, 105, 105, 64));
	m_view->setAttribute(Qt::WA_TranslucentBackground);

	const auto lay = new QHBoxLayout(this);

	lay->addWidget(m_view);

	setLayout(lay);

	m_view->show();
}

editor::graph_editor::~graph_editor(){}
