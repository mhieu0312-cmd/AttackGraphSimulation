#include "Graph.h"

#include <cctype>
#include <unordered_set>
#include <stdexcept>

using namespace std;

void Graph::requireNode(int id) const {
    if (getNode(id) == nullptr) {
        throw invalid_argument("Khong co node ID " + to_string(id));
    }
}

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

void Graph::addNode(Node node) {
    if (node.getID() < 0 || getNode(node.getID()) != nullptr) {
        throw invalid_argument("Node ID khong hop le");
    }

    const string name = node.getName();
    bool emptyName = true;
    for (unsigned char character : name) {
        if (isspace(character) == 0) {
            emptyName = false;
            break;
        }
    }
    if (emptyName) {
        throw invalid_argument("Node name rong");
    }

    if (node.getAssets() < 0) {
        throw invalid_argument("Assets phai >= 0");
    }

    nodeTypeToString(node.getType());
    Nodes.push_back(node);
}

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
    if (relation != "HasSession" && relation != "AdminTo" && relation != "MemberOf" &&
        relation != "AccessTo") {
        throw invalid_argument("Relation khong hop le: " + relation);
    }

    Edges.push_back(edge);
}

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
        if (getEdge(id) == nullptr) {
            throw invalid_argument("Khong co edge ID " + to_string(id));
        }
    }

    for (int id : edgeIds) {
        blockEdge(id);
    }
}

GraphStatistics Graph::statistics() const {
    GraphStatistics result;
    result.nodes = Nodes.size();
    result.edges = Edges.size();

    for (const Edge& edge : Edges) {
        if (edge.isBlocked()) {
            result.blockedEdges++;
        } else {
            result.activeEdges++;
        }
    }

    return result;
}

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

    // tinh ca canh da chan khi xet node co lap
    unordered_set<int> incident;
    for (const Edge& edge : Edges) {
        incident.insert(edge.getFrom());
        incident.insert(edge.getTo());
    }
    unordered_set<int> reachable;
    for (const Node& node : Nodes) {
        if (node.getType() == NodeType::ENTRY) {
            for (int id : dfs(node.getID())) {
                reachable.insert(id);
            }
        }
    }
    for (const Node& node : Nodes) {
        if (incident.count(node.getID()) == 0) {
            result.push_back("Node co lap ID " + to_string(node.getID()));
        }
        if (node.getType() == NodeType::TARGET && reachable.count(node.getID()) == 0) {
            result.push_back("TARGET khong reachable tu ENTRY: ID " + to_string(node.getID()));
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
        if (visited.count(current) != 0) {
            continue;
        }
        visited.insert(current);
        order.push_back(current);
        vector<const Edge*> edges = getOutGoingEdge(current);
        // day nguoc de giu thu tu duyet
        for (size_t i = edges.size(); i > 0; --i) {
            int next = edges[i - 1]->getTo();
            if (visited.count(next) == 0) {
                stack.push_back(next);
            }
        }
    }
    return order;
}
