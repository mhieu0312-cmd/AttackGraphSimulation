#include "MinCut.h"

#include <algorithm>
#include <limits>
#include <queue>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

using namespace std;

namespace {
struct ResidualArc {
    size_t from;
    size_t to;
    int64_t capacity;
};

int64_t checkedAdd(int64_t a, int64_t b) {
    if (b < 0 || a > numeric_limits<int64_t>::max() - b) {
        throw overflow_error("MinCut: int64_t capacity/flow overflow");
    }
    return a + b;
}

vector<bool> bfsResidual(const vector<ResidualArc>& arcs, const vector<vector<size_t>>& outgoing,
                         size_t source, vector<size_t>& parent) {
    fill(parent.begin(), parent.end(), numeric_limits<size_t>::max());
    vector<bool> reached(outgoing.size(), false);
    queue<size_t> pending;
    reached[source] = true;
    pending.push(source);

    while (!pending.empty()) {
        size_t current = pending.front();
        pending.pop();
        for (size_t edgeId : outgoing[current]) {
            const ResidualArc& edge = arcs[edgeId];
            if (edge.capacity > 0 && !reached[edge.to]) {
                reached[edge.to] = true;
                parent[edge.to] = edgeId;
                pending.push(edge.to);
            }
        }
    }
    return reached;
}
}

MinCutResult minimumSTCut(const Graph& graph, int source, int target) {
    if (graph.getNode(source) == nullptr) {
        throw invalid_argument("MinCut: source ID khong ton tai: " + to_string(source));
    }
    if (graph.getNode(target) == nullptr) {
        throw invalid_argument("MinCut: target ID khong ton tai: " + to_string(target));
    }
    if (source == target) {
        throw invalid_argument("MinCut: source phai khac target");
    }

    MinCutResult result;
    vector<int> originalReachable = graph.dfs(source);
    if (find(originalReachable.begin(), originalReachable.end(), target) ==
        originalReachable.end()) {
        return result;
    }

    // id khong phai vi tri trong mang
    unordered_map<int, size_t> index;
    for (const Node& node : graph.getNodes()) {
        size_t next = index.size();
        index.emplace(node.getID(), next);
    }
    size_t nodeCount = index.size();
    size_t sourceIndex = index.at(source);
    size_t targetIndex = index.at(target);
    vector<ResidualArc> arcs;
    vector<vector<size_t>> outgoing(nodeCount);

    for (const Edge& edge : graph.getEdges()) {
        if (edge.isBlocked()) {
            continue;
        }
        int64_t capacity = edge.getCapacity();
        if (capacity < 0) {
            throw invalid_argument("MinCut: capacity phai >= 0");
        }
        size_t from = index.at(edge.getFrom());
        size_t to = index.at(edge.getTo());

        // cap cung thuan va nguoc nam lien nhau
        size_t forward = arcs.size();
        arcs.push_back({from, to, capacity});
        arcs.push_back({to, from, 0});
        outgoing[from].push_back(forward);
        outgoing[to].push_back(forward + 1);
    }

    vector<size_t> parent(nodeCount, numeric_limits<size_t>::max());
    vector<bool> reachable = bfsResidual(arcs, outgoing, sourceIndex, parent);
    while (reachable[targetIndex]) {
        int64_t amount = numeric_limits<int64_t>::max();
        for (size_t node = targetIndex; node != sourceIndex; node = arcs[parent[node]].from) {
            amount = min(amount, arcs[parent[node]].capacity);
        }
        result.maxFlow = checkedAdd(result.maxFlow, amount);

        for (size_t node = targetIndex; node != sourceIndex; node = arcs[parent[node]].from) {
            size_t edgeId = parent[node];
            size_t reverseId;
            if (edgeId % 2 == 0) {
                reverseId = edgeId + 1;
            } else {
                reverseId = edgeId - 1;
            }
            arcs[edgeId].capacity -= amount;
            arcs[reverseId].capacity = checkedAdd(arcs[reverseId].capacity, amount);
        }
        reachable = bfsResidual(arcs, outgoing, sourceIndex, parent);
    }

    // capacity bang 0 van can chan
    for (const Edge& edge : graph.getEdges()) {
        if (!edge.isBlocked() && reachable[index.at(edge.getFrom())] &&
            !reachable[index.at(edge.getTo())]) {
            result.edgeIds.push_back(edge.getID());
            result.minCutCapacity = checkedAdd(result.minCutCapacity, edge.getCapacity());
        }
    }
    if (result.maxFlow != result.minCutCapacity) {
        throw logic_error("MinCut: max-flow khac min-cut capacity");
    }

    // thu cat ket noi, khong sua graph goc
    unordered_set<int> cut(result.edgeIds.begin(), result.edgeIds.end());
    unordered_set<int> visited{source};
    queue<int> pending;
    pending.push(source);
    while (!pending.empty()) {
        int current = pending.front();
        pending.pop();
        for (const Edge* edge : graph.getOutGoingEdge(current)) {
            int next = edge->getTo();
            if (cut.count(edge->getID()) == 0 && visited.count(next) == 0) {
                visited.insert(next);
                pending.push(next);
            }
        }
    }
    if (visited.count(target) != 0) {
        throw logic_error("MinCut: tap canh khong ngat duoc S-T");
    }
    return result;
}
