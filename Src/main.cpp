#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <climits>
#include <algorithm>

using namespace std;

// Cấu trúc thông tin cạnh kết nối
struct Edge {
    int id;
    int u, v;
    int capacity; // Dung lượng / Chi phí ngắt kết nối
    int flow;
    bool blocked;
};

// Cấu trúc thông tin Node
struct NodeInfo {
    int id;
    string name;
};

class Defender {
private:
    int numNodes;
    vector<Edge> edges;
    vector<vector<int>> adj; // Lưu chỉ số cạnh

    // BFS tìm đường tăng luồng trên đồ thị thặng dư
    bool bfsResidual(int src, int target, vector<vector<int>>& capacityMatrix, vector<int>& parent) {
        fill(parent.begin(), parent.end(), -1);
        parent[src] = src;
        queue<pair<int, int>> q;
        q.push({src, INT_MAX});

        while (!q.empty()) {
            int curr = q.front().first;
            int flow = q.front().second;
            q.pop();

            for (int next = 0; next < numNodes; ++next) {
                if (parent[next] == -1 && capacityMatrix[curr][next] > 0) {
                    parent[next] = curr;
                    int new_flow = min(flow, capacityMatrix[curr][next]);
                    if (next == target) return true;
                    q.push({next, new_flow});
                }
            }
        }
        return false;
    }

    // DFS tìm tập hợp các Node thuộc tập S (liên thông từ Source sau khi đã cạn luồng)
    void dfsReachable(int curr, const vector<vector<int>>& capacityMatrix, vector<bool>& visited) {
        visited[curr] = true;
        for (int next = 0; next < numNodes; ++next) {
            if (capacityMatrix[curr][next] > 0 && !visited[next]) {
                dfsReachable(next, capacityMatrix, visited);
            }
        }
    }

public:
    Defender(int nodes) : numNodes(nodes) {
        adj.resize(nodes);
    }

    void addEdge(int id, int u, int v, int capacity = 1) {
        edges.push_back({id, u, v, capacity, 0, false});
        adj[u].push_back(edges.size() - 1);
    }

    // 1. Kiểm tra liên thông cơ bản (BFS)
    bool isReachable(int src, int target) {
        vector<bool> visited(numNodes, false);
        queue<int> q;
        q.push(src);
        visited[src] = true;

        while (!q.empty()) {
            int curr = q.front();
            q.pop();

            if (curr == target) return true;

            for (int edgeIdx : adj[curr]) {
                const auto& edge = edges[edgeIdx];
                if (!edge.blocked && !visited[edge.v]) {
                    visited[edge.v] = true;
                    q.push(edge.v);
                }
            }
        }
        return false;
    }

    // 2. Thuật toán Min-Cut chuẩn (Edmonds-Karp Max-Flow Min-Cut)
    vector<int> suggestMinCutEdges(int src, int target) {
        // Khởi tạo ma trận dung lượng thặng dư
        vector<vector<int>> residualCap(numNodes, vector<int>(numNodes, 0));
        for (const auto& edge : edges) {
            if (!edge.blocked) {
                residualCap[edge.u][edge.v] += edge.capacity;
            }
        }

        vector<int> parent(numNodes);

        // BƯỚC A: Tìm luồng cực đại (Max Flow)
        while (bfsResidual(src, target, residualCap, parent)) {
            int path_flow = INT_MAX;
            for (int v = target; v != src; v = parent[v]) {
                int u = parent[v];
                path_flow = min(path_flow, residualCap[u][v]);
            }

            for (int v = target; v != src; v = parent[v]) {
                int u = parent[v];
                residualCap[u][v] -= path_flow;
                residualCap[v][u] += path_flow;
            }
        }

        // BƯỚC B: Xác định tập S (các đỉnh còn đến được từ Source trên đồ thị thặng dư)
        vector<bool> visitedFromSource(numNodes, false);
        dfsReachable(src, residualCap, visitedFromSource);

        // BƯỚC C: Tìm các cạnh đi từ tập S sang tập T (đây chính là Min-Cut Set)
        vector<int> cutEdgeIds;
        for (const auto& edge : edges) {
            if (!edge.blocked) {
                if (visitedFromSource[edge.u] && !visitedFromSource[edge.v]) {
                    cutEdgeIds.push_back(edge.id);
                }
            }
        }

        return cutEdgeIds;
    }

    // Chặn tập hợp các cạnh Min-Cut
    void blockEdges(const vector<int>& edgeIds) {
        for (int id : edgeIds) {
            for (auto& edge : edges) {
                if (edge.id == id) {
                    edge.blocked = true;
                    cout << "   [PATCH SUCCESS] Da ngat ket noi Canh ID [" << id << "]" << endl;
                    break;
                }
            }
        }
    }
};

int main() {
    cout << "==========================================================" << endl;
    cout << "  DEMO AI DEFENDER - EDMONDS-KARP MIN-CUT SIMULATION      " << endl;
    cout << "==========================================================" << endl;

    vector<NodeInfo> nodes = {
        {0, "Hacker (External Internet)"},
        {1, "Perimeter Firewall"},
        {2, "DMZ Web Server"},
        {3, "DMZ Mail Server"},
        {4, "Internal Firewall"},
        {5, "HR Workstation"},
        {6, "IT Admin Workstation"},
        {7, "Active Directory (AD)"},
        {8, "Database Gateway"},
        {9, "Core Database Server (TARGET)"}
    };

    Defender defender(10);

    // Thêm các cạnh nối (ID_Canh, Node_Tu, Node_Den, Chi_Phi_Cut)
    defender.addEdge(201, 0, 1, 10); // External -> Perimeter Firewall
    defender.addEdge(202, 1, 2, 5);  // Firewall -> DMZ Web
    defender.addEdge(203, 1, 3, 5);  // Firewall -> DMZ Mail
    defender.addEdge(204, 2, 4, 5);  // DMZ Web -> Internal FW
    defender.addEdge(205, 3, 4, 5);  // DMZ Mail -> Internal FW
    defender.addEdge(206, 4, 5, 3);  // Internal FW -> HR
    defender.addEdge(207, 4, 6, 3);  // Internal FW -> IT Admin
    defender.addEdge(208, 5, 7, 2);  // HR -> AD
    defender.addEdge(209, 6, 7, 2);  // IT Admin -> AD
    defender.addEdge(210, 7, 8, 8);  // AD -> DB Gateway
    defender.addEdge(211, 8, 9, 10); // Gateway -> Core DB

    int Source = 0; // Hacker
    int Target = 9; // Core Database

    // BƯỚC 1: PENTEST
    cout << "\n[BUOC 1] PENTEST: Kiem tra duong tan cong tu ["
         << nodes[Source].name << "] den [" << nodes[Target].name << "]..." << endl;

    if (defender.isReachable(Source, Target)) {
        cout << "=> CANH BAO NGUY HIEM: Red Team CO THE xam nhap va chiem Quyen Core Database!" << endl;
        cout << "=> Trang thai lien thong: Reachable = TRUE" << endl;
    } else {
        cout << "=> KET QUA: He thong an toan." << endl;
    }

    // BƯỚC 2: AI DEFENDER TÍNH TOÁN THUẬT TOÁN MIN-CUT CHUẨN
    cout << "\n[BUOC 2] DEFENDER: AI tinh toan Lat cat Cuc tieu (Min-Cut Set)..." << endl;
    vector<int> minCutEdges = defender.suggestMinCutEdges(Source, Target);

    if (!minCutEdges.empty()) {
        cout << "=> DE XUAT PHONG THU OPTIMAL: Can ngat [" << minCutEdges.size() << "] chot chan sau:" << endl;
        for (int id : minCutEdges) {
            cout << "   + Cắt Cạnh ID: [" << id << "]" << endl;
        }
    } else {
        cout << "=> Khong tim thay lat cat phu hop." << endl;
    }

    // BƯỚC 3: PHÒNG THỦ (APPLY PATCHES)
    cout << "\n[BUOC 3] ACTION: Thuc hien khoi tao Cac Chot chan (Apply Patches)..." << endl;
    defender.blockEdges(minCutEdges);

    // BƯỚC 4: RE-TEST XÁC NHẬN KẾT QUẢ
    cout << "\n[BUOC 4] RE-TEST: Kiem tra lai tinh lien thong sau khi Block..." << endl;
    if (defender.isReachable(Source, Target)) {
        cout << "=> KET QUA: Hacker van con duong vong khac!" << endl;
    } else {
        cout << "=> THANH CONG HOAN TOAN: Hacker da bi ngan chan HOAN TOAN!" << endl;
        cout << "=> Trang thai lien thong: Reachable = FALSE" << endl;
    }

    cout << "\n==========================================================" << endl;
    return 0;
}
