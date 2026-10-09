#pragma once

#include "Node.h"
#include "Edge.h"

#include <cstdint>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;


struct GraphStatistics {
    int nodes = 0;
    int edges = 0;
    int activeEdges = 0;
    int blockedEdges = 0;
};

class Graph {
    // Danh sach node
    vector<Node> Nodes;

    // Danh sach edge
    vector<Edge> Edges;

    // Kiem tra node co ton tai
    void requireNode(int id) const;


public:

    void addNode(Node node);

    void addEdge(Edge edge);

    // TIM NODE / EDGE
    const Node* getNode(int id) const;

    const Edge* getEdge(int id) const;


    // Lay danh sach node
    const vector<Node>& getNodes() const {
        return Nodes;
    }

    // Lay danh sach edge
    const vector<Edge>& getEdges() const {
        return Edges;
    }

    // Lay cac node co the di den
    vector<int> getNeighbor(int id) const;

    // Lay cac edge di ra va chua bi block
    vector<const Edge*> getOutGoingEdge(int id) const;

    // Kiem tra edge da bi block chua
    bool isBlocked(int edgeId) const;

    // Block mot edge
    void blockEdge(int edgeId);

    // Mo lai mot edge
    void unblockEdge(int edgeId);

    // Block nhieu edge
    void blockEdges(
        const vector<int>& edgeIds
    );

    // THONG KE
    GraphStatistics statistics() const;

    // KIEM TRA DATASET
    vector<string> validationWarnings() const;
};