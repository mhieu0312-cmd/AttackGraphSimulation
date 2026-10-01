#include "Graph.h"
#include <algorithm>
#include <limits>
#include <queue>
#include <set>
#include <stdexcept>
#include <utility>

void Graph::requireNode(int id) const {
    if (!getNode(id)) throw std::invalid_argument("Khong co node ID " + std::to_string(id));
}
const Node* Graph::getNode(int id) const {
    auto it = nodeIndex.find(id);
    return it == nodeIndex.end() ? nullptr : &Nodes[it->second];
}
const Edge* Graph::getEdge(int id) const {
    auto it = edgeIndex.find(id);
    return it == edgeIndex.end() ? nullptr : &Edges[it->second];
}
void Graph::addNode(Node node) {
    if (node.getID() < 0 || getNode(node.getID()))
        throw std::invalid_argument("Node ID am hoac bi trung: " + std::to_string(node.getID()));
    if (node.getName().find_first_not_of(" \t\r\n") == std::string::npos || node.getAssets() < 0)
        throw std::invalid_argument("Node name rong hoac assets am");
    nodeTypeToString(node.getType());
    nodeIndex[node.getID()] = Nodes.size();
    Nodes.push_back(std::move(node));
}
void Graph::addEdge(Edge edge) {
    requireNode(edge.getFrom());
    requireNode(edge.getTo());
    if (edge.getWeight() < 0 || (edge.getCapacity() && *edge.getCapacity() < 0))
        throw std::invalid_argument("weight/capacity phai >= 0");
    const std::set<std::string> relations{"HasSession", "AdminTo", "MemberOf", "AccessTo"};
    if (!relations.count(edge.getRelation())) throw std::invalid_argument("Relation khong hop le");
    if (edge.id == -1) { // Tuong thich constructor cu; JSON bat buoc co ID ro rang.
        edge.id = 0;
        while (getEdge(edge.id)) {
            if (edge.id == std::numeric_limits<int>::max()) throw std::overflow_error("Het edge ID");
            ++edge.id;
        }
    }
    if (edge.id < 0 || getEdge(edge.id)) throw std::invalid_argument("Edge ID am hoac bi trung");
    edgeIndex[edge.id] = Edges.size();
    Neighbors[edge.getFrom()].push_back(Edges.size());
    Edges.push_back(std::move(edge));
}
std::vector<const Edge*> Graph::getOutGoingEdge(int id) const {
    requireNode(id);
    std::vector<const Edge*> result;
    auto it = Neighbors.find(id);
    if (it != Neighbors.end()) {
        for (auto index : it->second) if (!Edges[index].isBlocked()) result.push_back(&Edges[index]);
    }
    return result;
}
std::vector<int> Graph::getNeighbor(int id) const {
    std::vector<int> result;
    for (const auto* e : getOutGoingEdge(id)) {
        if (std::find(result.begin(), result.end(), e->getTo()) == result.end()) result.push_back(e->getTo());
    }
    return result;
}
bool Graph::isBlocked(int edgeId) const {
    const auto* e = getEdge(edgeId);
    if (!e) throw std::invalid_argument("Khong co edge ID " + std::to_string(edgeId));
    return e->isBlocked();
}
void Graph::blockEdge(int edgeId) { blockEdges({edgeId}); }
void Graph::unblockEdge(int edgeId) {
    isBlocked(edgeId); // Kiem tra ID truoc khi thay doi.
    Edges[edgeIndex.at(edgeId)].setBlocked(false);
}
void Graph::blockEdges(const std::vector<int>& edgeIds) {
    // Kiem tra ca batch truoc, tranh patch nua chung khi mot ID sai.
    for (int id : edgeIds) isBlocked(id);
    for (int id : edgeIds) Edges[edgeIndex.at(id)].setBlocked(true);
}
bool Graph::canMove(int edgeId, std::int64_t remainingToken) const {
    const auto* e = getEdge(edgeId);
    return e && !e->isBlocked() && remainingToken >= e->getWeight();
}
std::vector<int> Graph::bfs(int start) const {
    requireNode(start);
    std::vector<int> order;
    std::set<int> visited{start};
    std::queue<int> q;
    q.push(start);
    while (!q.empty()) {
        int u = q.front(); q.pop(); order.push_back(u);
        for (int v : getNeighbor(u)) if (visited.insert(v).second) q.push(v);
    }
    return order;
}
std::vector<int> Graph::dfs(int start) const {
    requireNode(start);
    std::vector<int> order, stack{start};
    std::set<int> visited;
    while (!stack.empty()) {
        int u = stack.back(); stack.pop_back();
        if (!visited.insert(u).second) continue;
        order.push_back(u);
        auto neighbors = getNeighbor(u);
        for (auto it = neighbors.rbegin(); it != neighbors.rend(); ++it)
            if (!visited.count(*it)) stack.push_back(*it);
    }
    return order;
}
GraphStatistics Graph::statistics() const {
    GraphStatistics s;
    s.nodes = Nodes.size(); s.edges = Edges.size();
    for (const auto& n : Nodes) {
        s.inDegree[n.getID()] = s.outDegree[n.getID()] = 0;
        ++s.nodeTypes[nodeTypeToString(n.getType())];
    }
    for (const auto& e : Edges) {
        ++s.relations[e.getRelation()];
        if (e.isBlocked()) ++s.blockedEdges;
        else { ++s.activeEdges; ++s.outDegree[e.getFrom()]; ++s.inDegree[e.getTo()]; }
    }
    return s;
}
PathValidation Graph::validatePath(int start, int target, const std::vector<int>& edgeIds) const {
    if (!getNode(start) || !getNode(target)) return {false, 0, "Node khong ton tai"};
    int current = start;
    std::int64_t cost = 0;
    for (int id : edgeIds) {
        const auto* e = getEdge(id);
        if (!e || e->isBlocked() || e->getFrom() != current)
            return {false, 0, "Canh thieu, blocked hoac sai chieu/thu tu"};
        if (cost > std::numeric_limits<std::int64_t>::max() - e->getWeight())
            return {false, 0, "Tong chi phi tran so"};
        cost += e->getWeight(); current = e->getTo();
    }
    if (current != target) return {false, 0, "Duong chua den target"};
    return {true, cost, "OK"};
}
std::vector<std::string> Graph::validationWarnings() const {
    std::vector<std::string> result;
    if (Nodes.empty()) result.push_back("Graph rong");
    std::set<int> reachable;
    bool hasEntry = false, hasTarget = false;
    for (const auto& n : Nodes) {
        if (n.getType() == NodeType::ENTRY) {
            hasEntry = true;
            auto order = bfs(n.getID()); reachable.insert(order.begin(), order.end());
        }
        if (n.getType() == NodeType::TARGET) hasTarget = true;
    }
    if (!hasEntry) result.push_back("Khong co node ENTRY");
    if (!hasTarget) result.push_back("Khong co node TARGET");
    auto s = statistics();
    for (const auto& n : Nodes) {
        int id = n.getID();
        if (s.inDegree[id] == 0 && s.outDegree[id] == 0)
            result.push_back("Node co lap tren graph active: " + std::to_string(id));
        if (hasEntry && n.getType() == NodeType::TARGET && !reachable.count(id))
            result.push_back("Target khong reachable tu ENTRY: " + std::to_string(id));
    }
    for (const auto& e : Edges) if (!e.getCapacity()) {
        result.push_back("Thieu capacity: chua the tinh weighted Min-Cut"); break;
    }
    return result;
}
