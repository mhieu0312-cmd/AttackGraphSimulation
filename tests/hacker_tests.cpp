#include "Dijkstra.h"
#include "LoadDataset.h"

#include <iostream>
#include <stdexcept>
#include <vector>
#include <string>

using namespace std;

namespace {

void check(bool condition, const string& message) {
    if (!condition) {
        throw runtime_error("FAIL: " + message);
    }
}

// Do thi mau nho phuc vu unit test doc lap
Graph createSmallGraph() {
    Graph g;
    g.addNode(Node(10, "Entry", NodeType::ENTRY, 0));
    g.addNode(Node(20, "Workstation", NodeType::ENDPOINT, 10));
    g.addNode(Node(30, "Server", NodeType::CRITICAL_SYSTEM, 50));
    g.addNode(Node(40, "Target", NodeType::TARGET, 100));

    // Canh 1: 10 -> 20 (cost 4)
    g.addEdge(Edge(10, 20, 4, "HasSession", false, 1, 2));
    // Canh 2: 20 -> 40 (cost 5)
    g.addEdge(Edge(20, 40, 5, "AccessTo", false, 2, 3));
    // Canh 3: 10 -> 40 (cost 12 - duong truc tiep nhung dat hon)
    g.addEdge(Edge(10, 40, 12, "AccessTo", false, 3, 4));
    // Canh 4: 20 -> 30 (cost 2)
    g.addEdge(Edge(20, 30, 2, "MemberOf", false, 4, 1));
    // Canh 5: 30 -> 40 (cost 2)
    g.addEdge(Edge(30, 40, 2, "AdminTo", false, 5, 5));

    return g;
}

// 1. Kiem tra Dijkstra co ban
void testDijkstraBasic() {
    Graph g = createSmallGraph();

    // 10 -> 20 -> 30 -> 40 (4 + 2 + 2 = 8, re hon 10 -> 20 -> 40 gia 9 va 10 -> 40 gia 12)
    PathResult res = dijkstra(g, 10, 40);
    check(res.reachable, "Dijkstra phai tim duoc duong");
    check(res.totalCost == 8, "Chi phi toi uu phai la 8 (10->20->30->40)");
    check(res.nodes == vector<int>({10, 20, 30, 40}), "Danh sach node phai la 10->20->30->40");
    check(res.edgeIds == vector<int>({1, 4, 5}), "Danh sach edgeIds phai la 1, 4, 5");

    // Chan canh 4 (20 -> 30) thi duong re nhat thanh 10 -> 20 -> 40 (cost 9)
    g.blockEdge(4);
    PathResult resBlocked = dijkstra(g, 10, 40);
    check(resBlocked.reachable, "Van con duong di sau khi chan canh 4");
    check(resBlocked.totalCost == 9, "Chi phi sau khi chan canh 4 phai la 9");
    check(resBlocked.nodes == vector<int>({10, 20, 40}), "Danh sach node sau khi doi duong phai la 10->20->40");
}

// 2. Kiem tra ham canMove
void testCanMove() {
    Graph g = createSmallGraph();
    // Edge 1: weight = 4, blocked = false
    check(canMove(g, 1, 10), "Token = 10 du di qua edge weight 4");
    check(canMove(g, 1, 4), "Token = 4 vua du di qua edge weight 4");
    check(!canMove(g, 1, 3), "Token = 3 khong du di qua edge weight 4");
    check(!canMove(g, 1, -1), "Token am khong duoc phep");

    // Chan edge 1
    g.blockEdge(1);
    check(!canMove(g, 1, 100), "Edge da bi block thi du bao nhieu token cung khong di duoc");

    // Edge khong ton tai
    check(!canMove(g, 999, 100), "Edge ID khong ton tai phai tra ve false");
}

// 3. Kiem tra cac trang thai Budget cua Hacker
void testBudgetStates() {
    Graph g = createSmallGraph();
    // Duong toi uu co cost = 8

    // Case 1: Du token -> SUCCESS
    AttackResult a1 = hackerSimulation(g, 10, 40, 15);
    check(a1.status == AttackStatus::SUCCESS, "Budget 15 phai SUCCESS");
    check(a1.remainingToken.has_value() && *a1.remainingToken == 7, "Token con lai phai la 15 - 8 = 7");

    // Case 2: Vua khit token -> SUCCESS
    AttackResult a2 = hackerSimulation(g, 10, 40, 8);
    check(a2.status == AttackStatus::SUCCESS, "Budget 8 vua du phai SUCCESS");
    check(a2.remainingToken.has_value() && *a2.remainingToken == 0, "Token con lai phai la 0");

    // Case 3: Thieu token -> OVER_BUDGET
    AttackResult a3 = hackerSimulation(g, 10, 40, 7);
    check(a3.status == AttackStatus::OVER_BUDGET, "Budget 7 thieu token phai OVER_BUDGET");
    check(!a3.remainingToken.has_value(), "OVER_BUDGET khong co remainingToken");

    // Case 4: Budget am -> throw invalid_argument
    bool threw = false;
    try {
        hackerSimulation(g, 10, 40, -5);
    } catch (const invalid_argument&) {
        threw = true;
    }
    check(threw, "Budget am phai nem invalid_argument");
}

// 4. Kiem tra cac truong hop bien (Edge Cases)
void testEdgeCases() {
    Graph g = createSmallGraph();

    // Start == Target: cost 0, khong ton token
    AttackResult same = hackerSimulation(g, 10, 10, 0);
    check(same.status == AttackStatus::SUCCESS, "Start == Target phai SUCCESS voi budget 0");
    check(same.path.totalCost == 0, "Cost tai cho phai bang 0");
    check(same.path.nodes == vector<int>({10}), "Node tai cho phai la chinh no");
    check(same.path.edgeIds.empty(), "Edge IDs phai rong");

    // Di nguoc chieu (Target -> Entry) tren do thi co huong: NO_PATH
    AttackResult rev = hackerSimulation(g, 40, 10, 100);
    check(rev.status == AttackStatus::NO_PATH, "Di nguoc chieu do thi co huong phai NO_PATH");

    // Node khong ton tai
    bool threwNode = false;
    try {
        dijkstra(g, 999, 10);
    } catch (const invalid_argument&) {
        threwNode = true;
    }
    check(threwNode, "Node ID khong ton tai phai nem invalid_argument");
}

// 5. Test 10 Scenario thuc te tren dataset that (data/graph.json)
void testDatasetScenarios() {
    Graph g = loadDataset(string(DATASET_PATH));

    // Scenario 1: PC_LeTan (36) -> CustomerDB (3), Budget = 30 -> SUCCESS
    AttackResult s1 = hackerSimulation(g, 36, 3, 30);
    check(s1.status == AttackStatus::SUCCESS, "Scenario 1: Budget 30 phai SUCCESS");
    check(s1.path.totalCost == 28, "Scenario 1: Cost phai bang 28");
    check(*s1.remainingToken == 2, "Scenario 1: Token con lai phai la 2");
    check(s1.path.nodes == vector<int>({36, 32, 20, 7, 3}), "Scenario 1: Path nodes dung");

    // Scenario 2: PC_LeTan (36) -> CustomerDB (3), Budget = 27 -> OVER_BUDGET
    AttackResult s2 = hackerSimulation(g, 36, 3, 27);
    check(s2.status == AttackStatus::OVER_BUDGET, "Scenario 2: Budget 27 phai OVER_BUDGET");
    check(s2.path.totalCost == 28, "Scenario 2: Cost van la 28");

    // Scenario 3: PC_LeTan (36) -> SeverDC (1), Budget = 35 -> SUCCESS
    AttackResult s3 = hackerSimulation(g, 36, 1, 35);
    check(s3.status == AttackStatus::SUCCESS, "Scenario 3: 36 -> 1 phai SUCCESS");
    check(s3.path.totalCost == 29, "Scenario 3: Cost toi SeverDC phai la 29");
    check(*s3.remainingToken == 6, "Scenario 3: Token con lai phai la 6");
    check(s3.path.nodes == vector<int>({36, 26, 14, 4, 1}), "Scenario 3: Di qua PC_HR1 va User_LienHR");

    // Scenario 4: PC_LeTan (36) -> AdminDB (2), Budget = 35 -> SUCCESS
    AttackResult s4 = hackerSimulation(g, 36, 2, 35);
    check(s4.status == AttackStatus::SUCCESS, "Scenario 4: 36 -> 2 phai SUCCESS");
    check(s4.path.totalCost == 31, "Scenario 4: Cost toi AdminDB phai la 31");
    check(*s4.remainingToken == 4, "Scenario 4: Token con lai phai la 4");
    check(s4.path.nodes == vector<int>({36, 28, 15, 9, 2}), "Scenario 4: Di qua PC_KeToan1 va User_KeToanTruong");

    // Scenario 5: PC_LeTan (36) -> BackupServer (13), Budget = 35 -> SUCCESS
    AttackResult s5 = hackerSimulation(g, 36, 13, 35);
    check(s5.status == AttackStatus::SUCCESS, "Scenario 5: 36 -> 13 phai SUCCESS");
    check(s5.path.totalCost == 34, "Scenario 5: Cost toi BackupServer phai la 34");
    check(*s5.remainingToken == 1, "Scenario 5: Token con lai phai la 1");

    // Scenario 6: Patch Edge 48 (36 -> 32) -> Hacker tim duong vong moi toi CustomerDB (3)
    Graph gPatched = g;
    gPatched.blockEdge(48);
    AttackResult s6 = hackerSimulation(gPatched, 36, 3, 35);
    check(s6.status == AttackStatus::SUCCESS, "Scenario 6: Sau khi patch 48 van con duong vong");
    check(s6.path.totalCost == 29, "Scenario 6: Duong vong co cost la 29 (36->28->15->9->3)");
    check(s6.path.nodes == vector<int>({36, 28, 15, 9, 3}), "Scenario 6: Path vong qua KeToan");

    // Scenario 7: Voi Budget = 28, sau khi patch Edge 48 -> OVER_BUDGET
    AttackResult s7 = hackerSimulation(gPatched, 36, 3, 28);
    check(s7.status == AttackStatus::OVER_BUDGET, "Scenario 7: Budget 28 voi duong vong 29 phai OVER_BUDGET");

    // Scenario 8: Sau khi patch toan bo Min-Cut {47, 48, 49} -> NO_PATH
    Graph gCut = g;
    gCut.blockEdges({47, 48, 49});
    AttackResult s8 = hackerSimulation(gCut, 36, 3, 50);
    check(s8.status == AttackStatus::NO_PATH, "Scenario 8: Sau khi cat Min-Cut phai NO_PATH");

    // Scenario 9: Tan cong tu workstation noi bo: PC_Dev1 (32) -> BackupServer (13)
    AttackResult s9 = hackerSimulation(g, 32, 13, 30);
    check(s9.status == AttackStatus::SUCCESS, "Scenario 9: 32 -> 13 phai SUCCESS");
    check(s9.path.totalCost == 24, "Scenario 9: Cost tu 32 -> 13 phai la 24");
    check(*s9.remainingToken == 6, "Scenario 9: Token con lai phai la 6");

    // Scenario 10: Kiem tra canMove tren graph that voi Edge 48 (weight = 10)
    check(canMove(g, 48, 15), "Scenario 10: Edge 48 weight 10 voi 15 token phai true");
    check(canMove(g, 48, 10), "Scenario 10: Edge 48 weight 10 voi 10 token phai true");
    check(!canMove(g, 48, 9), "Scenario 10: Edge 48 weight 10 voi 9 token phai false");
    check(!canMove(gPatched, 48, 15), "Scenario 10: Edge 48 da bi block phai false");
}

} // namespace

int main(int argc, char* argv[]) {
    try {
        string group = (argc > 1) ? argv[1] : "all";
        bool ran = false;

        if (group == "all" || group == "basic") {
            testDijkstraBasic();
            cout << "dijkstra_basic PASS\n";
            ran = true;
        }
        if (group == "all" || group == "can_move") {
            testCanMove();
            cout << "can_move PASS\n";
            ran = true;
        }
        if (group == "all" || group == "budget") {
            testBudgetStates();
            cout << "budget_states PASS\n";
            ran = true;
        }
        if (group == "all" || group == "edge_cases") {
            testEdgeCases();
            cout << "edge_cases PASS\n";
            ran = true;
        }
        if (group == "all" || group == "scenarios") {
            testDatasetScenarios();
            cout << "dataset_10_scenarios PASS\n";
            ran = true;
        }

        if (!ran) {
            cerr << "Unknown test group: " << group << '\n';
            return 1;
        }

        cout << "\n>>> ALL P2 HACKER TESTS PASSED SUCCESSFULLY! <<<\n";
        return 0;
    } catch (const exception& e) {
        cerr << "TEST EXCEPTION: " << e.what() << '\n';
        return 1;
    }
}
