#pragma once
#include "Graph.h"
#include "raylib.h"
#include <vector>

class GraphView {
public:
    struct NodePosition {
        int id;
        NodeType type;
        std::string name;
        Vector2 position;
    };
    void setGraph(const Graph& graph);
    void updateGraph(const Graph& graph, const std::vector<int>& attack, const std::vector<int>& cut);
    void setBounds(Rectangle area);
    void fit();
    void handleInput();
    void draw(Font font, bool edgeLabels, bool dimOthers, int selectedNode) const;
    int nodeAt(Vector2 mouse) const;
    const std::vector<NodePosition>& getPositions() const { return nodes; }
    Rectangle getBounds() const { return bounds; }
    Camera2D getCamera() const { return camera; }

private:
    std::vector<NodePosition> nodes;
    std::vector<Edge> edges;
    std::vector<int> attackIds;
    std::vector<int> cutIds;
    Rectangle bounds{0, 0, 1000, 500};
    Camera2D camera{{500, 250}, {460, 0}, 0, 1};
    bool panning = false;
    Vector2 positionOf(int id) const;
};
