#include "Defender.h"

#include <queue>
#include <set>
#include <stdexcept>

using namespace std;

bool Defender::isReachable(int source, int target) const {
    if (graph.getNode(source) == nullptr || graph.getNode(target) == nullptr) {
        throw invalid_argument("Connectivity: source/target khong ton tai");
    }

    set<int> visited;
    queue<int> q;
    visited.insert(source);
    q.push(source);

    while (!q.empty()) {
        int current = q.front();
        q.pop();

        if (current == target) {
            return true;
        }

        for (const Edge* edge : graph.getOutGoingEdge(current)) {
            int nextNode = edge->getTo();

            if (visited.count(nextNode) == 0) {
                visited.insert(nextNode);
                q.push(nextNode);
            }
        }
    }

    return false;
}

int Defender::suggestCutEdge(int source, int target) const {

    if (!isReachable(source, target) || source == target) {
        return -1;
    }

    // thu tren ban sao cua graph

    Graph trial = graph;
    Defender probe(trial);

    for (const Edge& edge : graph.getEdges()) {
        if (edge.isBlocked()) {
            continue;
        }

        trial.blockEdge(edge.getID());
        bool disconnected = !probe.isReachable(source, target);
        trial.unblockEdge(edge.getID());

        if (disconnected) {
            return edge.getID();
        }
    }

    return -1;
}

void Defender::blockEdge(int edgeId) {
    graph.blockEdge(edgeId);
}
