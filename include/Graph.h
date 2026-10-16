#pragma once

#include "Node.h"
#include "Edge.h"

#include <string>
#include <vector>

using namespace std;

struct GraphStatistics {
    int nodes = 0;
    int edges = 0;
    int activeEdges = 0;
    int blockedEdges = 0;
};

class Graph {

    vector<Node> Nodes;

    vector<Edge> Edges;

    void requireNode(int id) const;

  public:
    void addNode(Node node);
    void addEdge(Edge edge);

    const Node* getNode(int id) const;
    const Edge* getEdge(int id) const;

    const vector<Node>& getNodes() const {
        return Nodes;
    }

    const vector<Edge>& getEdges() const {
        return Edges;
    }

    vector<int> getNeighbor(int id) const;

    // chi lay canh di ra chua bi chan
    vector<const Edge*> getOutGoingEdge(int id) const;

    vector<int> dfs(int start) const;

    bool isBlocked(int edgeId) const;

    void blockEdge(int edgeId);

    void unblockEdge(int edgeId);

    void blockEdges(const vector<int>& edgeIds);

    GraphStatistics statistics() const;

    vector<string> validationWarnings() const;
};
