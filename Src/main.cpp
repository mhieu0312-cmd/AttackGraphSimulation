#include <iostream>
#include <string>
#include <vector>
#include <queue>

using namespace std;

// Cấu trúc lưu trữ thông tin cạnh trong đồ thị
struct Edge {
    int id;
    int u;
    int v;
    bool blocked;
};

// Cấu trúc tên gợi nhớ cho Node trong mô phỏng
struct NodeInfo {
    int id;
    string name;
};

// Lớp Defender xử lý kiểm tra liên thông và thuật toán Min-Cut
class Defender {
private:
    int numNodes;
    vector<Edge> edges;

public:
    Defender(int nodes) : numNodes(nodes) {}

    // Thêm cạnh nối giữa 2 node
    void addEdge(int id, int u, int v) {
        edges.push_back({ id, u, v, false });
    }

    // Kiểm tra tính liên thông bằng thuật toán Duyệt theo chiều rộng (BFS)
    bool isReachable(int src, int target) {
        vector<bool> visited(numNodes, false);
        queue<int> q;

        q.push(src);
        visited[src] = true;

        while (!q.empty()) {
            int curr = q.front();
            q.pop();

            if (curr == target) return true;

            for (const auto& edge : edges) {
                if (edge.blocked) continue;
                if (edge.u == curr && !visited[edge.v]) {
                    visited[edge.v] = true;
                    q.push(edge.v);
                }
            }
        }
        return false;
    }

    // Gợi ý cạnh cần cắt (Min-Cut/Bridge) để ngăn chặn tấn công
    int suggestCutEdge(int src, int target) {
        for (auto& edge : edges) {
            if (edge.blocked) continue;

            // Thử chặn tạm thời cạnh này
            edge.blocked = true;

            // Nếu sau khi chặn, target không còn liên thông -> đây là chốt chặn tối ưu
            if (!isReachable(src, target)) {
                edge.blocked = false; // Hoàn trả trạng thái
                return edge.id;
            }

            edge.blocked = false; // Hoàn trả trạng thái
        }
        return -1;
    }

    // Chặn hoàn toàn kết nối tại cạnh chỉ định
    void blockEdge(int edgeId) {
        for (auto& edge : edges) {
            if (edge.id == edgeId) {
                edge.blocked = true;
                break;
            }
        }
    }
};

int main() {
    cout << "==========================================================" << endl;
    cout << "    DEMO AI DEFENDER - ATTACK GRAPH & MIN-CUT SIMULATION  " << endl;
    cout << "==========================================================" << endl;

    // Danh sách 10 Node đại diện cho hạ tầng Doanh nghiệp
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

    // Thêm các cạnh nối giữa các vùng mạng (ID_Canh, Node_Tu, Node_Den)
    defender.addEdge(201, 0, 1); // External -> Perimeter Firewall
    defender.addEdge(202, 1, 2); // Firewall -> DMZ Web Server
    defender.addEdge(203, 1, 3); // Firewall -> DMZ Mail Server
    defender.addEdge(204, 2, 4); // DMZ Web -> Internal Firewall
    defender.addEdge(205, 3, 4); // DMZ Mail -> Internal Firewall
    defender.addEdge(206, 4, 5); // Internal FW -> HR Workstation
    defender.addEdge(207, 4, 6); // Internal FW -> IT Admin
    defender.addEdge(208, 5, 7); // HR -> Active Directory
    defender.addEdge(209, 6, 7); // IT Admin -> Active Directory
    defender.addEdge(210, 7, 8); // AD -> Database Gateway
    defender.addEdge(211, 8, 9); // Gateway -> Core Database Server

    int Source = 0; // Hacker
    int Target = 9; // Core Database Server

    // BƯỚC 1: PENTEST - QUÉT KẾT NỐI BAN ĐẦU
    cout << "\n[BUOC 1] PENTEST: Kiem tra duong tan cong tu ["
        << nodes[Source].name << "] den [" << nodes[Target].name << "]..." << endl;

    if (defender.isReachable(Source, Target)) {
        cout << "=> CANH BAO NGUY HIEM: Red Team CO THE xam nhap va chiem Quyen Core Database!" << endl;
        cout << "=> Trang thai lien thong: Reachable = TRUE" << endl;
    }
    else {
        cout << "=> KET QUA: He thong an toan." << endl;
    }

    // BƯỚC 2: AI DEFENDER PHÂN TÍCH THUẬT TOÁN MIN-CUT
    cout << "\n[BUOC 2] DEFENDER: AI dang quet ma tran do thi va tinh toan Min-Cut..." << endl;
    int cutEdgeId = defender.suggestCutEdge(Source, Target);

    if (cutEdgeId != -1) {
        cout << "=> DE XUAT PHONG THU OPTIMAL: Can ngat ket noi tai Canh ID [" << cutEdgeId << "]" << endl;
        cout << "=> Giai thich: Day la chot chan chi mang giup co lap Target voi chi phi thap nhat." << endl;
    }

    // BƯỚC 3: PHÒNG THỦ (APPLY PATCH)
    cout << "\n[BUOC 3] ACTION: Thuc hien khoi tao Chot chan (Apply Patch)..." << endl;
    defender.blockEdge(cutEdgeId);

    // BƯỚC 4: RE-TEST XÁC NHẬN KẾT QUẢ
    cout << "\n[BUOC 4] RE-TEST: Kiem tra lai tinh lien thong sau khi Block..." << endl;
    if (defender.isReachable(Source, Target)) {
        cout << "=> KET QUA: Hacker van con duong vong khac." << endl;
    }
    else {
        cout << "=> THANH CONG CHUYEN DEI: Hacker da bi ngan chan HOAN TOAN!" << endl;
        cout << "=> Trang thai lien thong: Reachable = FALSE" << endl;
    }

    cout << "\n==========================================================" << endl;
    return 0;
}
