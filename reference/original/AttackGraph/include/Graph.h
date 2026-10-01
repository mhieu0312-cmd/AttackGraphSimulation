//
// Created by mhieu on 9/16/2026.
//

#ifndef ATTACKGRAPH_GRAPH_H
#define ATTACKGRAPH_GRAPH_H


#include <vector>
#include <unordered_map>

#include "Node.h"
#include "Edge.h"

using namespace std;

class Graph {
private:
    vector<Node> Nodes;
    vector<Edge> Edges;

    unordered_map<int, vector<int>> Neighbors;

public:

    // GraphAPI
    void addNode(Node node);
    void addEdge(Edge edge);

    Node* getNode(int id);

    vector<int> getNeighbor(int id);

    vector<Edge*> getOutGoingEdge(int id);
};

#endif