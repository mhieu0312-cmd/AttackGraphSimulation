//
// Created by mhieu on 9/16/2026.
//

#include "Graph.h"
void Graph::addNode(Node node) {
    Nodes.push_back(node);
}

void Graph::addEdge(Edge edge) {
    Edges.push_back(edge);

    Neighbors[edge.getFrom()].push_back(edge.getTo());
}

Node* Graph::getNode(int id) {

    for (Node& node : Nodes) {
        if (node.getID() == id) {
            return &node;
        }
    }

    return nullptr;
}

vector<int> Graph::getNeighbor(int id) {
    return Neighbors[id];
}

vector<Edge*> Graph::getOutGoingEdge(int id) {

    vector<Edge*> result;

    for (Edge& edge : Edges) {

        if (edge.getFrom() == id) {
            result.push_back(&edge);
        }
    }

    return result;
}