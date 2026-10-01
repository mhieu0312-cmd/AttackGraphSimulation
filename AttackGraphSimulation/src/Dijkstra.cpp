#include "Dijkstra.h"
#include <algorithm>
#include <limits>
#include <ostream>
#include <stdexcept>
#include <unordered_map>

namespace {
// Giu cach chon dinh nho nhat O(V^2) trong prototype Nguoi 2.
std::size_t minDistance(const std::vector<std::int64_t>& distance,
                        const std::vector<bool>& visited) {
    std::size_t index = distance.size();
    auto best = std::numeric_limits<std::int64_t>::max();
    for (std::size_t i = 0; i < distance.size(); ++i) {
        if (!visited[i] && distance[i] < best) { best = distance[i]; index = i; }
    }
    return index;
}
}

PathResult dijkstra(const Graph& graph, int start, int target) {
    if (!graph.getNode(start) || !graph.getNode(target))
        throw std::invalid_argument("Dijkstra: source/target khong ton tai");
    const auto& nodes = graph.getNodes();
    const auto n = nodes.size();
    const auto inf = std::numeric_limits<std::int64_t>::max();
    std::unordered_map<int, std::size_t> index;
    for (std::size_t i = 0; i < n; ++i) index[nodes[i].getID()] = i;
    std::vector<std::int64_t> distance(n, inf);
    std::vector<bool> visited(n, false);
    std::vector<int> parentEdge(n, -1);
    distance[index.at(start)] = 0;
    for (std::size_t count = 0; count < n; ++count) {
        auto u = minDistance(distance, visited);
        if (u == n) break;
        visited[u] = true;
        if (nodes[u].getID() == target) break;
        for (const auto* e : graph.getOutGoingEdge(nodes[u].getID())) {
            auto v = index.at(e->getTo());
            if (visited[v]) continue;
            if (distance[u] > inf - e->getWeight()) throw std::overflow_error("Dijkstra cost overflow");
            auto newCost = distance[u] + e->getWeight();
            if (newCost < distance[v]) {
                distance[v] = newCost;
                parentEdge[v] = e->getID();
            }
        }
    }
    if (distance[index.at(target)] == inf) return {};
    PathResult result;
    result.reachable = true;
    result.totalCost = distance[index.at(target)];
    int current = target;
    result.nodes.push_back(current);
    while (current != start) {
        const auto* e = graph.getEdge(parentEdge[index.at(current)]);
        if (!e) throw std::logic_error("Dijkstra: parent edge khong hop le");
        result.edgeIds.push_back(e->getID());
        current = e->getFrom(); result.nodes.push_back(current);
    }
    std::reverse(result.nodes.begin(), result.nodes.end());
    std::reverse(result.edgeIds.begin(), result.edgeIds.end());
    return result;
}
AttackResult hackerSimulation(const Graph& graph, int start, int target, std::int64_t budget) {
    if (budget < 0) throw std::invalid_argument("Budget phai >= 0");
    AttackResult result;
    result.initialBudget = budget;
    result.path = dijkstra(graph, start, target);
    if (!result.path.reachable) result.status = AttackStatus::NO_PATH;
    else if (result.path.totalCost > budget) result.status = AttackStatus::OVER_BUDGET;
    else {
        result.status = AttackStatus::SUCCESS;
        result.remainingToken = budget - result.path.totalCost;
    }
    return result;
}
void printPath(const PathResult& path, std::ostream& out) {
    for (std::size_t i = 0; i < path.nodes.size(); ++i) {
        if (i) out << " -> ";
        out << path.nodes[i];
    }
}
void printAttack(const AttackResult& attack, std::ostream& out) {
    out << "Budget khoi tao: " << attack.initialBudget << '\n';
    if (!attack.path.reachable) { out << "NO_PATH: khong con duong di.\n"; return; }
    out << "Path: "; printPath(attack.path, out);
    out << "\nCost: " << attack.path.totalCost << "\nEdge IDs:";
    for (int id : attack.path.edgeIds) out << ' ' << id;
    if (attack.status == AttackStatus::SUCCESS)
        out << "\nSUCCESS: token con lai = " << *attack.remainingToken << '\n';
    else out << "\nOVER_BUDGET: van co duong nhung vuot ngan sach.\n";
}
