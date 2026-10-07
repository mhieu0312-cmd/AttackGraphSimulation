#include "Graph.h"

#include <stdexcept>

using namespace std;


// =========================
// KIEM TRA NODE
// =========================

void Graph::requireNode(int id) const {
    if (getNode(id) == nullptr) {
        throw invalid_argument("Khong co node ID " + to_string(id));
    }
}


// =========================
// TIM NODE / EDGE
// =========================

const Node* Graph::getNode(int id) const {

    for (const Node& node : Nodes) {
        if (node.getID() == id) {
            return &node;
        }
    }

    return nullptr;
}

const Edge* Graph::getEdge(int id) const {

    for (const Edge& edge : Edges) {
        if (edge.getID() == id) {
            return &edge;
        }
    }

    return nullptr;
}


// =========================
// THEM NODE
// =========================

void Graph::addNode(Node node) {

    if (node.getID() < 0 || getNode(node.getID()) != nullptr) {
        throw invalid_argument("Node ID khong hop le");
    }

    if (node.getName().empty()) {
        throw invalid_argument("Node name rong");
    }

    Nodes.push_back(node);
}


// =========================
// THEM EDGE
// =========================

void Graph::addEdge(Edge edge) {

    requireNode(edge.getFrom());
    requireNode(edge.getTo());

    if (edge.getID() < 0 || getEdge(edge.getID()) != nullptr) {
        throw invalid_argument("Edge ID khong hop le");
    }

    if (edge.getWeight() < 0) {
        throw invalid_argument("Weight phai >= 0");
    }

    if (edge.getCapacity() < 0) {
        throw invalid_argument("Capacity phai >= 0");
    }

    Edges.push_back(edge);
}


// =========================
// TIM CAC EDGE DI RA
// =========================

vector<const Edge*> Graph::getOutGoingEdge(int id) const {

    requireNode(id);

    vector<const Edge*> result;

    for (const Edge& edge : Edges) {

        if (edge.getFrom() == id && !edge.isBlocked()) {
            result.push_back(&edge);
        }
    }

    return result;
}


// =========================
// TIM NODE KE
// =========================

vector<int> Graph::getNeighbor(int id) const {

    requireNode(id);

    vector<int> result;

    for (const Edge& edge : Edges) {

        if (edge.getFrom() == id && !edge.isBlocked()) {
            result.push_back(edge.getTo());
        }
    }

    return result;
}


// =========================
// BLOCK / UNBLOCK EDGE
// =========================

bool Graph::isBlocked(int edgeId) const {

    const Edge* edge = getEdge(edgeId);

    if (edge == nullptr) {
        throw invalid_argument("Khong co edge ID " + to_string(edgeId));
    }

    return edge->isBlocked();
}

void Graph::blockEdge(int edgeId) {

    for (Edge& edge : Edges) {

        if (edge.getID() == edgeId) {
            edge.setBlocked(true);
            return;
        }
    }

    throw invalid_argument("Khong co edge ID " + to_string(edgeId));
}

void Graph::unblockEdge(int edgeId) {

    for (Edge& edge : Edges) {

        if (edge.getID() == edgeId) {
            edge.setBlocked(false);
            return;
        }
    }

    throw invalid_argument("Khong co edge ID " + to_string(edgeId));
}

void Graph::blockEdges(const vector<int>& edgeIds) {

    for (int id : edgeIds) {
        blockEdge(id);
    }
}


// =========================
// THONG KE GRAPH
// =========================

GraphStatistics Graph::statistics() const {

    GraphStatistics result;

    result.nodes = Nodes.size();
    result.edges = Edges.size();

    for (const Edge& edge : Edges) {

        if (edge.isBlocked()) {
            result.blockedEdges++;
        }
        else {
            result.activeEdges++;
        }
    }

    return result;
}


// =========================
// KIEM TRA DATASET
// =========================

vector<string> Graph::validationWarnings() const {

    vector<string> result;

    if (Nodes.empty()) {
        result.push_back("Graph rong");
        return result;
    }

    bool hasEntry = false;
    bool hasTarget = false;

    for (const Node& node : Nodes) {

        if (node.getType() == NodeType::ENTRY) {
            hasEntry = true;
        }

        if (node.getType() == NodeType::TARGET) {
            hasTarget = true;
        }
    }

    if (!hasEntry) {
        result.push_back("Khong co node ENTRY");
    }

    if (!hasTarget) {
        result.push_back("Khong co node TARGET");
    }

    return result;
}