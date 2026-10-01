#include "Defender.h"

Defender::Defender(int nodes) : numNodes(nodes) {
    adj.resize(nodes);
}

void Defender::addEdge(int id, int u, int v) {
    edges.push_back({ id, u, v, false });
    adj[u].push_back(edges.size() - 1); // Lưu vị trí cạnh
}

// 1. Dùng BFS để kiểm tra xem Source có đến được Target không
bool Defender::isReachable(int source, int target) {
    std::vector<bool> visited(numNodes, false);
    std::queue<int> q;

    q.push(source);
    visited[source] = true;

    while (!q.empty()) {
        int u = q.front();
        q.pop();

        if (u == target) return true; // Đã đến được Target

        for (int edgeIdx : adj[u]) {
            Edge& e = edges[edgeIdx];
            // Nếu cạnh chưa bị block và node đích chưa thăm
            if (!e.blocked && !visited[e.v]) {
                visited[e.v] = true;
                q.push(e.v);
            }
        }
    }
    return false; // Không thể đến được Target
}

// 2. Tìm cạnh quan trọng nằm trên đường đi để cắt (Min-Cut Simulator)
int Defender::suggestCutEdge(int source, int target) {
    for (size_t i = 0; i < edges.size(); ++i) {
        if (!edges[i].blocked) {
            // Thử chặn tạm thời cạnh này
            edges[i].blocked = true;
            // Kiểm tra xem sau khi chặn, S còn tới được T không
            if (!isReachable(source, target)) {
                edges[i].blocked = false; // Mở lại để trả kết quả
                return edges[i].id; // Đây chính là cạnh chí mạng cần cắt!
            }
            edges[i].blocked = false; // Hoàn tác thử nghiệm
        }
    }
    return -1; // Không tìm thấy cạnh đơn lẻ nào (hoặc đã bị chặn sẵn)
}

// 3. Vá lỗi (Set blocked = true)
void Defender::blockEdge(int edgeId) {
    for (auto& e : edges) {
        if (e.id == edgeId) {
            e.blocked = true;
            std::cout << "[BLUE TEAM] Da thuc hien PATCH: Chan thanh cong Canh ID " << edgeId << std::endl;
            return;
        }
    }
}