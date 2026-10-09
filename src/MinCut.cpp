#include "MinCut.h"

#include <algorithm>
#include <limits>
#include <queue>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

namespace {
struct ResidualArc {
    std::size_t from;
    std::size_t to;
    std::int64_t capacity;
};

std::int64_t checkedAdd(std::int64_t a, std::int64_t b) {
    if (b < 0 || a > std::numeric_limits<std::int64_t>::max() - b) {
        throw std::overflow_error("MinCut: int64_t capacity/flow overflow");
    }
    return a + b;
}
}

MinCutResult minimumSTCut(const Graph& graph, int source, int target) {
    if (!graph.getNode(source)) {
        throw std::invalid_argument("MinCut: source ID khong ton tai: " + std::to_string(source));
    }
    if (!graph.getNode(target)) {
        throw std::invalid_argument("MinCut: target ID khong ton tai: " + std::to_string(target));
    }
    if (source == target) {
        throw std::invalid_argument("MinCut: source phai khac target");
    }
    MinCutResult result;
    const auto originalReachable = graph.dfs(source);
    if (std::find(originalReachable.begin(), originalReachable.end(), target) == originalReachable.end()) {
        return result; // No active topological path: no patch is needed.
    }

    std::unordered_map<int, std::size_t> index;
    for (const auto& node : graph.getNodes()) {
        const auto next = index.size();
        index.emplace(node.getID(), next);
    }
    const auto n = index.size();
    const auto s = index.at(source);
    const auto t = index.at(target);
    std::vector<ResidualArc> arcs;
    std::vector<std::vector<std::size_t>> outgoing(n);
    for (const auto& edge : graph.getEdges()) {
        if (edge.isBlocked()) continue;
        const std::int64_t capacity = edge.getCapacity();
        if (capacity < 0) {
            throw std::invalid_argument("MinCut: capacity phai >= 0");
        }
        const auto u = index.at(edge.getFrom());
        const auto v = index.at(edge.getTo());
        // Each original edge gets its own pair. XOR 1 finds the paired arc,
        // even for parallel edges, opposite original edges and self-loops.
        const auto forward = arcs.size();
        arcs.push_back({u, v, capacity});
        arcs.push_back({v, u, 0});
        outgoing[u].push_back(forward);
        outgoing[v].push_back(forward + 1);
    }

    const auto none = std::numeric_limits<std::size_t>::max();
    std::vector<std::size_t> parent(n, none);
    auto bfs = [&]() {
        std::fill(parent.begin(), parent.end(), none);
        std::vector<bool> reached(n, false);
        std::queue<std::size_t> queue;
        reached[s] = true;
        queue.push(s);
        while (!queue.empty()) {
            const auto u = queue.front();
            queue.pop();
            for (const auto id : outgoing[u]) {
                const auto& arc = arcs[id];
                if (arc.capacity > 0 && !reached[arc.to]) {
                    reached[arc.to] = true;
                    parent[arc.to] = id;
                    queue.push(arc.to);
                }
            }
        }
        return reached;
    };

    auto reachable = bfs();
    while (reachable[t]) {
        auto amount = std::numeric_limits<std::int64_t>::max();
        for (auto v = t; v != s; v = arcs[parent[v]].from) {
            amount = std::min(amount, arcs[parent[v]].capacity);
        }
        result.maxFlow = checkedAdd(result.maxFlow, amount);
        for (auto v = t; v != s; v = arcs[parent[v]].from) {
            const auto id = parent[v];
            arcs[id].capacity -= amount;
            arcs[id ^ 1].capacity = checkedAdd(arcs[id ^ 1].capacity, amount);
        }
        reachable = bfs();
    }

    // Include ALL active original S->T edges, even capacity-zero edges:
    // zero residual capacity does not mean the attack edge is blocked.
    for (const auto& edge : graph.getEdges()) {
        if (!edge.isBlocked() && reachable[index.at(edge.getFrom())] &&
            !reachable[index.at(edge.getTo())]) {
            result.edgeIds.push_back(edge.getID());
            result.minCutCapacity = checkedAdd(result.minCutCapacity, edge.getCapacity());
        }
    }
    if (result.maxFlow != result.minCutCapacity) {
        throw std::logic_error("MinCut: max-flow khac min-cut capacity");
    }

    // Verify topological disconnection without changing or patching Graph.
    const std::unordered_set<int> cut(result.edgeIds.begin(), result.edgeIds.end());
    std::unordered_set<int> visited{source};
    std::queue<int> queue;
    queue.push(source);
    while (!queue.empty()) {
        const int u = queue.front();
        queue.pop();
        for (const auto* edge : graph.getOutGoingEdge(u)) {
            if (!cut.count(edge->getID()) && visited.insert(edge->getTo()).second) {
                queue.push(edge->getTo());
            }
        }
    }
    if (visited.count(target)) {
        throw std::logic_error("MinCut: tap canh khong ngat duoc S-T");
    }
    return result;
}
