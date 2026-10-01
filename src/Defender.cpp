#include "Defender.h"

#include <queue>
#include <set>
#include <stdexcept>

using namespace std;


// =========================
// KIEM TRA DUONG DI
// =========================

bool Defender::isReachable(int source, int target) const {

    // Kiem tra source va target co ton tai
    if (graph.getNode(source) == nullptr ||
        graph.getNode(target) == nullptr) {

        throw invalid_argument(
            "Connectivity: source/target khong ton tai"
        );
    }

    set<int> visited;
    queue<int> q;

    visited.insert(source);
    q.push(source);

    // Dung BFS de tim target
    while (!q.empty()) {

        int current = q.front();
        q.pop();

        // Da den target
        if (current == target) {
            return true;
        }

        // Duyet cac edge chua bi block
        for (const Edge* edge : graph.getOutGoingEdge(current)) {

            int nextNode = edge->getTo();

            if (visited.insert(nextNode).second) {
                q.push(nextNode);
            }
        }
    }

    // Khong tim thay duong den target
    return false;
}


// =========================
// GOI Y EDGE CAN BLOCK
// =========================

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

        // Bo qua edge da bi block
        if (edge.isBlocked()) {
            continue;
        }

        // Thu block edge
        trial.blockEdge(edge.getID());

        bool disconnected =
            !probe.isReachable(source, target);

        // Tra edge ve trang thai ban dau
        trial.unblockEdge(edge.getID());

        // Block edge nay lam mat duong S -> T
        if (disconnected) {
            return edge.getID();
        }
    }

    // Khong co mot edge don le nao cat duoc graph
    return -1;
}


// =========================
// BLOCK EDGE
// =========================

void Defender::blockEdge(int edgeId) {

    graph.blockEdge(edgeId);
}