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
// DIEU KIEN DI CHUYEN (CAN MOVE)
// =========================

bool canMove(
    const Graph& graph,
    int edgeId,
    int64_t remainingToken
) {
    // Token am khong the di chuyen
    if (remainingToken < 0) {
        return false;
    }

    const Edge* edge = graph.getEdge(edgeId);

    // Canh khong ton tai hoac da bi Defender chan
    if (edge == nullptr || edge->isBlocked()) {
        return false;
    }

    // Token con lai phai du chi tra weight cua canh
    return remainingToken >= edge->getWeight();
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


// =========================
// IN CHI TIET (TRUC QUAN KEM TEN NODE)
// =========================

void printPathDetailed(
    const Graph& graph,
    const PathResult& path,
    ostream& out
) {
    if (!path.reachable || path.nodes.empty()) {
        out << "(khong co duong di)\n";
        return;
    }

    if (path.nodes.size() == 1) {
        const Node* node = graph.getNode(path.nodes[0]);
        out << "  [1] "
            << (node ? node->getName() : ("Node " + to_string(path.nodes[0])))
            << " (ID " << path.nodes[0] << ") [Xuat phat tai dich]\n";
        return;
    }

    for (size_t i = 0; i < path.edgeIds.size(); ++i) {
        int u = path.nodes[i];
        int v = path.nodes[i + 1];
        int edgeId = path.edgeIds[i];

        const Node* uNode = graph.getNode(u);
        const Node* vNode = graph.getNode(v);
        const Edge* edge = graph.getEdge(edgeId);

        string uName = uNode ? uNode->getName() : ("Node " + to_string(u));
        string vName = vNode ? vNode->getName() : ("Node " + to_string(v));
        string rel = edge ? edge->getRelation() : "Edge";
        int64_t w = edge ? edge->getWeight() : 0;

        out << "  [" << (i + 1) << "] " << uName << " (ID " << u << ")\n";
        out << "      ---(" << rel << ", Cost: " << w << ", Edge ID: " << edgeId << ")--->\n";

        if (i + 1 == path.edgeIds.size()) {
            out << "  [" << (i + 2) << "] " << vName << " (ID " << v << ") [TARGET]\n";
        }
    }
}

void printAttackDetailed(
    const Graph& graph,
    const AttackResult& attack,
    ostream& out
) {
    out << "Budget khoi tao: " << attack.initialBudget << '\n';

    if (!attack.path.reachable) {
        out << "Trang thai: NO_PATH (Khong co duong di den muc tieu)\n";
        return;
    }

    out << "Lo trinh tan cong chi tiet:\n";
    printPathDetailed(graph, attack.path, out);

    out << "Tong chi phi: " << attack.path.totalCost << " Token\n";
    out << "Edge IDs:";
    for (int id : attack.path.edgeIds) {
        out << ' ' << id;
    }
    out << '\n';

    if (attack.status == AttackStatus::SUCCESS) {
        out << "Ket luan: SUCCESS (Hacker tiep can muc tieu thanh cong)\n";
        out << "Token con lai: " << *attack.remainingToken << '\n';
    } else {
        out << "Ket luan: OVER_BUDGET (Phat hien duong di nhung thieu "
            << (attack.path.totalCost - attack.initialBudget) << " Token)\n";
    }
}