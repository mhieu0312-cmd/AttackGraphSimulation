#include "Defender.h"
#include <queue>
#include <set>
#include <stdexcept>

bool Defender::isReachable(int source, int target) const {
    if (!graph.getNode(source) || !graph.getNode(target))
        throw std::invalid_argument("Connectivity: source/target khong ton tai");
    std::set<int> visited{source};
    std::queue<int> q;
    q.push(source);
    while (!q.empty()) {
        int u = q.front(); q.pop();
        if (u == target) return true;
        for (const auto* e : graph.getOutGoingEdge(u)) {
            if (visited.insert(e->getTo()).second) q.push(e->getTo());
        }
    }
    return false;
}
int Defender::suggestCutEdge(int source, int target) const {
    if (!isReachable(source, target) || source == target) return -1;
    // Thu tren ban sao: de xuat khong duoc thay doi graph dang chay.
    Graph trial = graph;
    Defender probe(trial);
    for (const auto& e : graph.getEdges()) {
        if (e.isBlocked()) continue;
        trial.blockEdge(e.getID());
        bool disconnects = !probe.isReachable(source, target);
        trial.unblockEdge(e.getID());
        if (disconnects) return e.getID();
    }
    return -1;
}
void Defender::blockEdge(int edgeId) { graph.blockEdge(edgeId); }
