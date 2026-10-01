#include "Graph.h"

#include <algorithm>
#include <limits>
#include <queue>
#include <set>
#include <stdexcept>
#include <utility>

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
    auto it = nodeIndex.find(id);

    if (it == nodeIndex.end()) {
        return nullptr;
    }

    return &Nodes[it->second];
}

const Edge* Graph::getEdge(int id) const {
    auto it = edgeIndex.find(id);

    if (it == edgeIndex.end()) {
        return nullptr;
    }

    return &Edges[it->second];
}


// =========================
// THEM NODE
// =========================

void Graph::addNode(Node node) {

    // ID khong duoc am hoac trung
    if (node.getID() < 0 || getNode(node.getID()) != nullptr) {
        throw invalid_argument(
            "Node ID am hoac bi trung: " + to_string(node.getID())
        );
    }

    // Ten khong duoc rong
    if (node.getName().find_first_not_of(" \t\r\n") == string::npos) {
        throw invalid_argument("Node name rong");
    }

    // Assets khong duoc am
    if (node.getAssets() < 0) {
        throw invalid_argument("Assets phai >= 0");
    }

    // Kiem tra NodeType hop le
    nodeTypeToString(node.getType());

    // Luu vi tri node
    nodeIndex[node.getID()] = Nodes.size();

    Nodes.push_back(move(node));
}


// =========================
// THEM EDGE
// =========================

void Graph::addEdge(Edge edge) {

    // Hai dau edge phai ton tai
    requireNode(edge.getFrom());
    requireNode(edge.getTo());

    // Weight khong duoc am
    if (edge.getWeight() < 0) {
        throw invalid_argument("Weight phai >= 0");
    }

    // Capacity neu co thi khong duoc am
    if (edge.getCapacity() && *edge.getCapacity() < 0) {
        throw invalid_argument("Capacity phai >= 0");
    }

    // Cac relation cho phep
    const set<string> relations = {
        "HasSession",
        "AdminTo",
        "MemberOf",
        "AccessTo"
    };

    if (!relations.count(edge.getRelation())) {
        throw invalid_argument("Relation khong hop le");
    }

    // Neu constructor cu khong co ID thi tu tao ID
    if (edge.id == -1) {
        edge.id = 0;

        while (getEdge(edge.id) != nullptr) {

            if (edge.id == numeric_limits<int>::max()) {
                throw overflow_error("Het edge ID");
            }

            ++edge.id;
        }
    }

    // ID edge khong duoc am hoac trung
    if (edge.id < 0 || getEdge(edge.id) != nullptr) {
        throw invalid_argument("Edge ID am hoac bi trung");
    }

    // Luu vi tri edge
    edgeIndex[edge.id] = Edges.size();

    // Luu edge vao danh sach ke cua node from
    Neighbors[edge.getFrom()].push_back(Edges.size());

    Edges.push_back(move(edge));
}


// =========================
// LAY EDGE DI RA
// =========================

vector<const Edge*> Graph::getOutGoingEdge(int id) const {

    requireNode(id);

    vector<const Edge*> result;

    auto it = Neighbors.find(id);

    if (it == Neighbors.end()) {
        return result;
    }

    for (size_t index : it->second) {

        // Chi lay edge chua bi block
        if (!Edges[index].isBlocked()) {
            result.push_back(&Edges[index]);
        }
    }

    return result;
}


// =========================
// LAY NODE KE
// =========================

vector<int> Graph::getNeighbor(int id) const {

    vector<int> result;

    for (const Edge* edge : getOutGoingEdge(id)) {

        int nextNode = edge->getTo();

        // Khong them node trung
        if (find(result.begin(), result.end(), nextNode) == result.end()) {
            result.push_back(nextNode);
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
        throw invalid_argument(
            "Khong co edge ID " + to_string(edgeId)
        );
    }

    return edge->isBlocked();
}

void Graph::blockEdge(int edgeId) {
    blockEdges({edgeId});
}

void Graph::unblockEdge(int edgeId) {

    // Kiem tra edge truoc
    isBlocked(edgeId);

    Edges[edgeIndex.at(edgeId)].setBlocked(false);
}

void Graph::blockEdges(const vector<int>& edgeIds) {

    // Kiem tra tat ca ID truoc
    for (int id : edgeIds) {
        isBlocked(id);
    }

    // Sau do moi block
    for (int id : edgeIds) {
        Edges[edgeIndex.at(id)].setBlocked(true);
    }
}


// =========================
// KIEM TRA HACKER CO DI DUOC
// =========================

bool Graph::canMove(int edgeId, int64_t remainingToken) const {

    const Edge* edge = getEdge(edgeId);

    if (edge == nullptr) {
        return false;
    }

    if (edge->isBlocked()) {
        return false;
    }

    return remainingToken >= edge->getWeight();
}


// =========================
// BFS
// =========================

vector<int> Graph::bfs(int start) const {

    requireNode(start);

    vector<int> order;
    set<int> visited;
    queue<int> q;

    visited.insert(start);
    q.push(start);

    while (!q.empty()) {

        int current = q.front();
        q.pop();

        order.push_back(current);

        for (int next : getNeighbor(current)) {

            if (visited.insert(next).second) {
                q.push(next);
            }
        }
    }

    return order;
}


// =========================
// DFS
// =========================

vector<int> Graph::dfs(int start) const {

    requireNode(start);

    vector<int> order;
    vector<int> stack;
    set<int> visited;

    stack.push_back(start);

    while (!stack.empty()) {

        int current = stack.back();
        stack.pop_back();

        // Node da tham thi bo qua
        if (!visited.insert(current).second) {
            continue;
        }

        order.push_back(current);

        vector<int> neighbors = getNeighbor(current);

        // Duyet nguoc de giu thu tu DFS
        for (auto it = neighbors.rbegin();
             it != neighbors.rend();
             ++it) {

            if (!visited.count(*it)) {
                stack.push_back(*it);
            }
        }
    }

    return order;
}


// =========================
// THONG KE GRAPH
// =========================

GraphStatistics Graph::statistics() const {

    GraphStatistics result;

    result.nodes = Nodes.size();
    result.edges = Edges.size();

    // Khoi tao thong ke node
    for (const Node& node : Nodes) {

        int id = node.getID();

        result.inDegree[id] = 0;
        result.outDegree[id] = 0;

        ++result.nodeTypes[
            nodeTypeToString(node.getType())
        ];
    }

    // Thong ke edge
    for (const Edge& edge : Edges) {

        ++result.relations[edge.getRelation()];

        if (edge.isBlocked()) {
            ++result.blockedEdges;
        }
        else {
            ++result.activeEdges;

            ++result.outDegree[edge.getFrom()];
            ++result.inDegree[edge.getTo()];
        }
    }

    return result;
}


// =========================
// KIEM TRA MOT DUONG DI
// =========================

PathValidation Graph::validatePath(
    int start,
    int target,
    const vector<int>& edgeIds
) const {

    // Kiem tra start va target
    if (getNode(start) == nullptr ||
        getNode(target) == nullptr) {

        return {
            false,
            0,
            "Node khong ton tai"
        };
    }

    int current = start;
    int64_t cost = 0;

    for (int edgeId : edgeIds) {

        const Edge* edge = getEdge(edgeId);

        // Edge phai ton tai va dung thu tu
        if (edge == nullptr ||
            edge->isBlocked() ||
            edge->getFrom() != current) {

            return {
                false,
                0,
                "Canh thieu, blocked hoac sai chieu/thu tu"
            };
        }

        // Kiem tra tran so
        if (cost >
            numeric_limits<int64_t>::max()
            - edge->getWeight()) {

            return {
                false,
                0,
                "Tong chi phi tran so"
            };
        }

        cost += edge->getWeight();

        current = edge->getTo();
    }

    // Ket thuc nhung chua den target
    if (current != target) {
        return {
            false,
            0,
            "Duong chua den target"
        };
    }

    return {
        true,
        cost,
        "OK"
    };
}


// =========================
// KIEM TRA DATASET
// =========================

vector<string> Graph::validationWarnings() const {

    vector<string> result;

    if (Nodes.empty()) {
        result.push_back("Graph rong");
    }

    set<int> reachable;

    bool hasEntry = false;
    bool hasTarget = false;

    // Tim cac node co the den tu ENTRY
    for (const Node& node : Nodes) {

        if (node.getType() == NodeType::ENTRY) {

            hasEntry = true;

            vector<int> order = bfs(node.getID());

            reachable.insert(
                order.begin(),
                order.end()
            );
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

    GraphStatistics stats = statistics();

    // Kiem tra node co lap va target khong reachable
    for (const Node& node : Nodes) {

        int id = node.getID();

        if (stats.inDegree[id] == 0 &&
            stats.outDegree[id] == 0) {

            result.push_back(
                "Node co lap tren graph active: "
                + to_string(id)
            );
        }

        if (hasEntry &&
            node.getType() == NodeType::TARGET &&
            !reachable.count(id)) {

            result.push_back(
                "Target khong reachable tu ENTRY: "
                + to_string(id)
            );
        }
    }

    // Min-Cut can capacity
    for (const Edge& edge : Edges) {

        if (!edge.getCapacity()) {
            result.push_back(
                "Thieu capacity: chua the tinh weighted Min-Cut"
            );

            // Canh bao mot lan la du
            break;
        }
    }

    return result;
}