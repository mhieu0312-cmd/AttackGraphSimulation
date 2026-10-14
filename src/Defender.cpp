#include "Defender.h"

#include <queue>
#include <set>
#include <stdexcept>

using namespace std;

// KIEM TRA DUONG DI

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

// GOI Y EDGE CAN BLOCK

int Defender::suggestCutEdge(int source, int target) const {
    // Khong co duong di thi khong can block
    if (!isReachable(source, target) || source == target) {
        return -1;
    }

    // Tao ban sao de thu block
    // Khong lam thay doi graph chinh
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

    // Khong co mot edge don le nao cat duoc graph
    return -1;
}

// BLOCK EDGE

void Defender::blockEdge(int edgeId) {
    graph.blockEdge(edgeId);
}
