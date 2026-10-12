#include "GraphView.h"

#include <QFontMetrics>
#include <QGraphicsEllipseItem>
#include <QGraphicsPathItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsScene>
#include <QGraphicsSimpleTextItem>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QSignalBlocker>
#include <QWheelEvent>
#include <algorithm>
#include <cmath>

namespace {
const double radius = 22.0;

bool containsId(const std::vector<int>& ids, int id) {
    for (int value : ids) {
        if (value == id) return true;
    }
    return false;
}

QColor nodeColor(NodeType type) {
    switch (type) {
        case NodeType::ENTRY: return QColor("#4387cc");
        case NodeType::ENDPOINT: return QColor("#73869e");
        case NodeType::IDENTITY: return QColor("#8874b2");
        case NodeType::CRITICAL_SYSTEM: return QColor("#348c9a");
        case NodeType::TARGET: return QColor("#369d73");
    }
    return QColor("#73869e");
}

// Sap xep trong moi tang theo vi tri trung binh cua node lien quan o tang ben canh.
void orderLayer(std::vector<std::vector<int>>& layers, int layer, int neighborLayer,
                const Graph& graph) {
    std::vector<double> scores;
    for (int id : layers[layer]) {
        double sum = 0;
        int count = 0;
        for (const Edge& edge : graph.getEdges()) {
            int neighbor = -1;
            if (edge.getFrom() == id) neighbor = edge.getTo();
            else if (edge.getTo() == id) neighbor = edge.getFrom();
            for (size_t index = 0; index < layers[neighborLayer].size(); ++index) {
                if (layers[neighborLayer][index] == neighbor) {
                    sum += static_cast<double>(index);
                    ++count;
                }
            }
        }
        scores.push_back(count == 0 ? static_cast<double>(scores.size()) : sum / count);
    }
    for (size_t pass = 0; pass < scores.size(); ++pass) {
        for (size_t index = 1; index < scores.size(); ++index) {
            if (scores[index - 1] > scores[index]) {
                std::swap(scores[index - 1], scores[index]);
                std::swap(layers[layer][index - 1], layers[layer][index]);
            }
        }
    }
}
}

GraphView::GraphView(QWidget* parent) : QGraphicsView(parent) {
    setObjectName("graphView");
    setScene(new QGraphicsScene(this));
    setBackgroundBrush(QColor("#0a0e14"));
    setFrameShape(QFrame::NoFrame);
    setRenderHint(QPainter::Antialiasing);
    setDragMode(QGraphicsView::ScrollHandDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorViewCenter);
}

QPointF GraphView::nodePosition(int id) const {
    for (const NodeDrawing& node : nodes) {
        if (node.id == id) return node.position;
    }
    return QPointF();
}

void GraphView::setGraph(const Graph& graph) {
    nodes.clear();
    edges.clear();
    attackEdgeIds.clear();
    cutEdgeIds.clear();
    scene()->clear();
    const NodeType types[]{NodeType::ENTRY, NodeType::ENDPOINT, NodeType::IDENTITY,
                           NodeType::CRITICAL_SYSTEM, NodeType::TARGET};
    const QStringList names{"ENTRY", "ENDPOINT", "IDENTITY", "CRITICAL SYSTEM", "TARGET"};
    std::vector<std::vector<int>> layers(5);
    size_t largestLayer = 1;
    for (int layer = 0; layer < 5; ++layer) {
        for (const Node& node : graph.getNodes()) {
            if (node.getType() == types[layer]) layers[layer].push_back(node.getID());
        }
        largestLayer = std::max(largestLayer, layers[layer].size());
    }
    for (int pass = 0; pass < 2; ++pass) {
        for (int layer = 1; layer < 5; ++layer) orderLayer(layers, layer, layer - 1, graph);
        for (int layer = 3; layer >= 0; --layer) orderLayer(layers, layer, layer + 1, graph);
    }
    const double height = static_cast<double>(largestLayer - 1) * 115.0;
    QFont nameFont;
    nameFont.setPixelSize(13);
    QFont idFont = nameFont;
    idFont.setBold(true);
    idFont.setPixelSize(16);
    for (int layer = 0; layer < 5; ++layer) {
        const double x = layer * 210.0;
        auto* heading = scene()->addSimpleText(names.at(layer), nameFont);
        heading->setBrush(QColor("#9db0c7"));
        heading->setPos(x - heading->boundingRect().width() / 2, -height / 2 - 85);
        for (size_t index = 0; index < layers[layer].size(); ++index) {
            const int id = layers[layer][index];
            const Node* node = graph.getNode(id);
            const double y = (static_cast<double>(index) -
                static_cast<double>(layers[layer].size() - 1) / 2.0) * 115.0;
            QPointF position(x, y);
            auto* circle = scene()->addEllipse(-radius, -radius, radius * 2, radius * 2);
            circle->setPos(position);
            circle->setZValue(3);
            circle->setData(0, id);
            circle->setData(1, QString("node"));
            circle->setFlag(QGraphicsItem::ItemIsSelectable);
            circle->setToolTip(QString::fromStdString(node->getName()) + "\nID: " +
                QString::number(id) + "\n" + QString::fromStdString(nodeTypeToString(node->getType())));
            auto* idLabel = new QGraphicsSimpleTextItem(QString::number(id), circle);
            QFont fittedFont = idFont;
            const int textWidth = QFontMetrics(idFont).horizontalAdvance(QString::number(id));
            if (textWidth > 38) fittedFont.setPixelSize(std::max(7, 16 * 38 / textWidth));
            idLabel->setFont(fittedFont);
            idLabel->setBrush(Qt::white);
            idLabel->setPos(-idLabel->boundingRect().width() / 2, -idLabel->boundingRect().height() / 2);
            idLabel->setAcceptedMouseButtons(Qt::NoButton);
            const QString fullName = QString::fromStdString(node->getName());
            auto* name = new QGraphicsSimpleTextItem(
                QFontMetrics(nameFont).elidedText(fullName, Qt::ElideRight, 170), circle);
            name->setFont(nameFont);
            name->setBrush(QColor("#d9e2ed"));
            name->setPos(-name->boundingRect().width() / 2, radius + 7);
            name->setAcceptedMouseButtons(Qt::NoButton);
            nodes.push_back({id, node->getType(), position, circle});
        }
    }
    for (const Edge& edge : graph.getEdges()) {
        const QPointF from = nodePosition(edge.getFrom());
        const QPointF to = nodePosition(edge.getTo());
        int ordinal = 0;
        int total = 0;
        for (const Edge& other : graph.getEdges()) {
            const bool samePair = (other.getFrom() == edge.getFrom() && other.getTo() == edge.getTo()) ||
                (other.getFrom() == edge.getTo() && other.getTo() == edge.getFrom());
            if (samePair) {
                if (other.getID() == edge.getID()) ordinal = total;
                ++total;
            }
        }
        double offset = (ordinal - (total - 1) / 2.0) * 32.0;
        if (edge.getFrom() > edge.getTo()) offset = -offset;
        QPainterPath path;
        if (edge.getFrom() == edge.getTo()) {
            path.moveTo(from + QPointF(15, -15));
            path.cubicTo(from + QPointF(90 + ordinal * 25, -110),
                         from + QPointF(-65, -110 - ordinal * 25), from + QPointF(-15, -15));
        } else if (std::abs(to.x() - from.x()) < 1.0) {
            path.moveTo(from + QPointF(radius, 0));
            const double side = from.x() + 95 + ordinal * 30;
            path.cubicTo(QPointF(side, from.y()), QPointF(side, to.y()), to + QPointF(radius, 0));
        } else {
            const double direction = to.x() > from.x() ? 1.0 : -1.0;
            const QPointF start = from + QPointF(direction * radius, 0);
            const QPointF end = to - QPointF(direction * radius, 0);
            if (std::abs(to.x() - from.x()) > 250) offset -= 65;
            path.moveTo(start);
            const double bend = std::abs(end.x() - start.x()) * 0.5;
            path.cubicTo(start + QPointF(direction * bend, offset),
                         end - QPointF(direction * bend, -offset), end);
        }
        auto* line = scene()->addPath(path);
        line->setData(0, edge.getID());
        line->setData(1, QString("edge"));
        const QString tooltip = QString("Edge %1: %2 -> %3\n%4\nWeight: %5 | Capacity: %6")
            .arg(edge.getID()).arg(edge.getFrom()).arg(edge.getTo())
            .arg(QString::fromStdString(edge.getRelation())).arg(edge.getWeight()).arg(edge.getCapacity());
        line->setToolTip(tooltip);
        const QPointF end = path.pointAtPercent(1.0);
        QPointF direction = end - path.pointAtPercent(0.98);
        const double length = std::hypot(direction.x(), direction.y());
        if (length > 0) direction /= length;
        const QPointF normal(-direction.y(), direction.x());
        QPolygonF triangle;
        triangle << end << end - direction * 11 + normal * 5 << end - direction * 11 - normal * 5;
        auto* arrow = scene()->addPolygon(triangle);
        arrow->setToolTip(tooltip);
        auto* label = scene()->addSimpleText(QString::fromStdString(edge.getRelation()) +
            QString("  w%1/c%2").arg(edge.getWeight()).arg(edge.getCapacity()), nameFont);
        const QPointF middle = path.pointAtPercent(0.5);
        label->setPos(middle.x() - label->boundingRect().width() / 2, middle.y() - 20);
        label->setToolTip(tooltip);
        label->setZValue(2);
        label->setAcceptedMouseButtons(Qt::NoButton);
        edges.push_back({edge.getID(), edge.getFrom(), edge.getTo(), edge.isBlocked(), tooltip, line, arrow, label});
    }
    refreshColors();
    scene()->setSceneRect(scene()->itemsBoundingRect().adjusted(-45, -45, 45, 45));
    fitGraph();
}

void GraphView::updateGraph(const Graph& graph, const std::vector<int>& attackIds,
                            const std::vector<int>& cutIds) {
    attackEdgeIds = attackIds;
    cutEdgeIds = cutIds;
    for (EdgeDrawing& drawing : edges) {
        const Edge* edge = graph.getEdge(drawing.id);
        if (edge != nullptr) drawing.blocked = edge->isBlocked();
    }
    refreshColors();
}

void GraphView::refreshColors() {
    const bool hasFocus = !attackEdgeIds.empty() || !cutEdgeIds.empty();
    for (EdgeDrawing& edge : edges) {
        const bool attack = containsId(attackEdgeIds, edge.id);
        const bool cut = containsId(cutEdgeIds, edge.id);
        QColor color("#697b92");
        double width = 1.6;
        if (attack) { color = QColor("#f5b04d"); width = 4.0; }
        if (cut) { color = QColor("#ce7af5"); width = 4.0; }
        if (edge.blocked) { color = QColor("#f07880"); width = 2.7; }
        QPen pen(color, width);
        if (edge.blocked) pen.setStyle(Qt::DashLine);
        const QString tooltip = edge.tooltip + (edge.blocked ? "\nState: blocked" : "\nState: active");
        edge.line->setToolTip(tooltip);
        edge.arrow->setToolTip(tooltip);
        edge.label->setToolTip(tooltip);
        edge.line->setPen(pen);
        edge.arrow->setBrush(color);
        edge.arrow->setPen(Qt::NoPen);
        edge.label->setBrush(color);
        const double opacity = dimUnrelated && hasFocus && !attack && !cut && !edge.blocked ? 0.16 : 1.0;
        edge.line->setOpacity(opacity);
        edge.arrow->setOpacity(opacity);
        edge.label->setOpacity(opacity);
        edge.label->setVisible(labelsVisible);
        edge.line->setZValue(attack || cut || edge.blocked ? 1 : 0);
        edge.arrow->setZValue(edge.line->zValue() + 0.1);
    }
    for (NodeDrawing& node : nodes) {
        bool onPath = false;
        for (const EdgeDrawing& edge : edges) {
            if (containsId(attackEdgeIds, edge.id) && (node.id == edge.from || node.id == edge.to)) {
                onPath = true;
            }
        }
        node.circle->setBrush(onPath ? QColor("#cc8b34") : nodeColor(node.type));
        node.circle->setPen(QPen(QColor("#e0e9f5"), 2.0));
    }
}

void GraphView::selectNode(int id) {
    const QSignalBlocker blocker(scene());
    scene()->clearSelection();
    for (const NodeDrawing& node : nodes) {
        if (node.id == id) {
            node.circle->setSelected(true);
            ensureVisible(node.circle, 30, 30);
        }
    }
}

void GraphView::setEdgeLabelsVisible(bool visible) {
    labelsVisible = visible;
    refreshColors();
}

void GraphView::setDimUnrelated(bool dim) {
    dimUnrelated = dim;
    refreshColors();
}

void GraphView::fitGraph() {
    resetTransform();
    if (!scene()->items().isEmpty()) fitInView(scene()->sceneRect(), Qt::KeepAspectRatio);
}

void GraphView::changeZoom(double factor) {
    const double current = transform().m11();
    const double next = std::max(0.02, std::min(5.0, current * factor));
    if (current > 0) scale(next / current, next / current);
}

void GraphView::zoomIn() { changeZoom(1.2); }
void GraphView::zoomOut() { changeZoom(1.0 / 1.2); }

void GraphView::wheelEvent(QWheelEvent* event) {
    if (event->angleDelta().y() > 0) zoomIn();
    else if (event->angleDelta().y() < 0) zoomOut();
    event->accept();
}

void GraphView::mousePressEvent(QMouseEvent* event) {
    QGraphicsItem* item = itemAt(event->pos());
    while (item != nullptr && item->data(1).toString() != "node") item = item->parentItem();
    QGraphicsView::mousePressEvent(event);
    if (event->button() == Qt::LeftButton && item != nullptr) emit nodeSelected(item->data(0).toInt());
}
