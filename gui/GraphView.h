#pragma once

#include "Graph.h"
#include <QGraphicsView>
#include <QPointF>
#include <vector>

class QGraphicsEllipseItem;
class QGraphicsPathItem;
class QGraphicsPolygonItem;
class QGraphicsSimpleTextItem;
class QWheelEvent;
class QMouseEvent;

class GraphView : public QGraphicsView {
    Q_OBJECT

public:
    explicit GraphView(QWidget* parent = nullptr);
    void setGraph(const Graph& graph);
    void updateGraph(const Graph& graph, const std::vector<int>& attackIds,
                     const std::vector<int>& cutIds);
    void selectNode(int id);
    void setEdgeLabelsVisible(bool visible);
    void setDimUnrelated(bool dim);
    void fitGraph();
    void zoomIn();
    void zoomOut();

signals:
    void nodeSelected(int id);

protected:
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    struct NodeDrawing {
        int id;
        NodeType type;
        QPointF position;
        QGraphicsEllipseItem* circle;
    };
    struct EdgeDrawing {
        int id;
        int from;
        int to;
        bool blocked;
        QString tooltip;
        QGraphicsPathItem* line;
        QGraphicsPolygonItem* arrow;
        QGraphicsSimpleTextItem* label;
    };

    std::vector<NodeDrawing> nodes;
    std::vector<EdgeDrawing> edges;
    std::vector<int> attackEdgeIds;
    std::vector<int> cutEdgeIds;
    bool labelsVisible = false;
    bool dimUnrelated = false;

    QPointF nodePosition(int id) const;
    void refreshColors();
    void changeZoom(double factor);
};
