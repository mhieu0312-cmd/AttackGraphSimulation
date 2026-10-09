#include "Graph.h"

#include <algorithm>
#include <cctype>
#include <unordered_set>
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

    const string name = node.getName();
    if (name.empty() || all_of(name.begin(), name.end(),
            [](unsigned char c) { return std::isspace(c) != 0; })) {
        throw invalid_argument("Node name rong");
    }

    // Reject invalid enum values even when nodes are added directly.
    nodeTypeToString(node.getType());
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

    const string relation = edge.getRelation();
    if (relation != "HasSession" && relation != "AdminTo" &&
        relation != "MemberOf" && relation != "AccessTo") {
        throw invalid_argument("Relation khong hop le: " + relation);
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

    // Isolation concerns the stored topology, including blocked edges.
    unordered_set<int> incident;
    for (const auto& edge : Edges) {
        incident.insert(edge.getFrom());
        incident.insert(edge.getTo());
    }
    unordered_set<int> reachable;
    for (const auto& node : Nodes) {
        if (node.getType() == NodeType::ENTRY) {
            for (int id : dfs(node.getID())) {
                reachable.insert(id);
            }
        }
    }
    for (const auto& node : Nodes) {
        if (incident.count(node.getID()) == 0) {
            result.push_back("Node co lap ID " + to_string(node.getID()));
        }
        if (node.getType() == NodeType::TARGET &&
            reachable.count(node.getID()) == 0) {
            result.push_back("TARGET khong reachable tu ENTRY: ID " +
                             to_string(node.getID()));
        }
    }

    return result;
}

vector<int> Graph::dfs(int start) const {
    requireNode(start);
    vector<int> order;
    vector<int> stack{start};
    unordered_set<int> visited;
    while (!stack.empty()) {
        const int current = stack.back();
        stack.pop_back();
        if (!visited.insert(current).second) {
            continue;
        }
        order.push_back(current);
        const auto edges = getOutGoingEdge(current);
        // Reverse push preserves the order in which outgoing edges were added.
        for (auto it = edges.rbegin(); it != edges.rend(); ++it) {
            if (visited.count((*it)->getTo()) == 0) {
                stack.push_back((*it)->getTo());
            }
        }
    }
    return order;
}
