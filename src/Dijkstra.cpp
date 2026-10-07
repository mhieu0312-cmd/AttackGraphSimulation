#include "Dijkstra.h"

#include <algorithm>
#include <limits>
#include <ostream>
#include <stdexcept>
#include <unordered_map>

using namespace std;


// =========================
// TIM NODE CO DISTANCE NHO NHAT
// =========================

namespace {

size_t minDistance(
    const vector<int64_t>& distance,
    const vector<bool>& visited
) {

    size_t index = distance.size();

    int64_t best = numeric_limits<int64_t>::max();

    for (size_t i = 0; i < distance.size(); ++i) {

        if (!visited[i] && distance[i] < best) {
            best = distance[i];
            index = i;
        }
    }

    return index;
}

}


// =========================
// DIJKSTRA
// =========================

PathResult dijkstra(
    const Graph& graph,
    int start,
    int target
) {

    // Kiem tra start va target
    if (graph.getNode(start) == nullptr ||
        graph.getNode(target) == nullptr) {

        throw invalid_argument(
            "Dijkstra: source/target khong ton tai"
        );
    }

    const vector<Node>& nodes = graph.getNodes();

    size_t n = nodes.size();

    int64_t inf = numeric_limits<int64_t>::max();

    // Map Node ID sang vi tri trong vector
    unordered_map<int, size_t> index;

    for (size_t i = 0; i < n; ++i) {
        index[nodes[i].getID()] = i;
    }

    // Khoi tao distance
    vector<int64_t> distance(n, inf);

    // Danh dau node da duyet
    vector<bool> visited(n, false);

    // Luu edge dung de di den moi node
    vector<int> parentEdge(n, -1);

    distance[index.at(start)] = 0;


    // Duyet tung node
    for (size_t count = 0; count < n; ++count) {

        size_t u = minDistance(distance, visited);

        // Khong con node nao di duoc
        if (u == n) {
            break;
        }

        visited[u] = true;

        // Da den target
        if (nodes[u].getID() == target) {
            break;
        }


        // Duyet cac edge di ra
        for (const Edge* edge :
             graph.getOutGoingEdge(nodes[u].getID())) {

            size_t v = index.at(edge->getTo());

            // Node da duyet thi bo qua
            if (visited[v]) {
                continue;
            }

            // Kiem tra tran so
            if (distance[u] >
                inf - edge->getWeight()) {

                throw overflow_error(
                    "Dijkstra cost overflow"
                );
            }

            int64_t newCost =
                distance[u] + edge->getWeight();

            // Tim duoc duong re hon
            if (newCost < distance[v]) {

                distance[v] = newCost;

                parentEdge[v] = edge->getID();
            }
        }
    }


    // Khong co duong den target
    if (distance[index.at(target)] == inf) {
        return {};
    }


    // Tao ket qua
    PathResult result;

    result.reachable = true;

    result.totalCost =
        distance[index.at(target)];


    // Truy nguoc duong di
    int current = target;

    result.nodes.push_back(current);


    while (current != start) {

        int edgeId =
            parentEdge[index.at(current)];

        const Edge* edge =
            graph.getEdge(edgeId);

        if (edge == nullptr) {
            throw logic_error(
                "Dijkstra: parent edge khong hop le"
            );
        }

        result.edgeIds.push_back(
            edge->getID()
        );

        current = edge->getFrom();

        result.nodes.push_back(current);
    }


    // Dao lai thanh start -> target
    reverse(
        result.nodes.begin(),
        result.nodes.end()
    );

    reverse(
        result.edgeIds.begin(),
        result.edgeIds.end()
    );

    return result;
}


// =========================
// MO PHONG HACKER
// =========================

AttackResult hackerSimulation(
    const Graph& graph,
    int start,
    int target,
    int64_t budget
) {

    // Budget khong duoc am
    if (budget < 0) {
        throw invalid_argument(
            "Budget phai >= 0"
        );
    }


    AttackResult result;

    result.initialBudget = budget;

    // Tim duong re nhat
    result.path =
        dijkstra(graph, start, target);


    // Khong co duong di
    if (!result.path.reachable) {

        result.status =
            AttackStatus::NO_PATH;
    }

    // Co duong nhung khong du token
    else if (result.path.totalCost > budget) {

        result.status =
            AttackStatus::OVER_BUDGET;
    }

    // Hacker di den target thanh cong
    else {

        result.status =
            AttackStatus::SUCCESS;

        result.remainingToken =
            budget - result.path.totalCost;
    }


    return result;
}


// =========================
// IN DUONG DI
// =========================

void printPath(
    const PathResult& path,
    ostream& out
) {

    for (size_t i = 0;
         i < path.nodes.size();
         ++i) {

        if (i > 0) {
            out << " -> ";
        }

        out << path.nodes[i];
    }
}


// =========================
// IN KET QUA ATTACK
// =========================

void printAttack(
    const AttackResult& attack,
    ostream& out
) {

    out << "Budget khoi tao: "
        << attack.initialBudget
        << '\n';


    // Khong co duong di
    if (!attack.path.reachable) {

        out << "NO_PATH: khong con duong di.\n";

        return;
    }


    out << "Path: ";

    printPath(attack.path, out);


    out << "\nCost: "
        << attack.path.totalCost;

    out << "\nEdge IDs:";


    for (int id : attack.path.edgeIds) {
        out << ' ' << id;
    }


    // Tan cong thanh cong
    if (attack.status == AttackStatus::SUCCESS) {

        out << "\nSUCCESS: token con lai = "
            << *attack.remainingToken
            << '\n';
    }

    // Co duong nhung vuot budget
    else {

        out << "\nOVER_BUDGET: "
               "van co duong nhung vuot ngan sach.\n";
    }
}