#include <iostream>
using namespace std;

// ===== BAN GIAO NGUOI 2: RED TEAM / HACKER =====
// Graph co toi da 40 node (du an thuc te khoang 38 node).
const int MAX_NODE = 40;
const int INF = 1000000000; // Dai dien cho "khong co duong di".

int cost[MAX_NODE][MAX_NODE];
bool blocked[MAX_NODE][MAX_NODE];

// Tim node CHUA XET co khoang cach nho nhat.
int minDistance(int distance[], bool visited[], int n) {
    int best = INF;
    int index = -1;

    for (int i = 0; i < n; i++) {
        if (!visited[i] && distance[i] < best) {
            best = distance[i];
            index = i;
        }
    }
    return index;
}

// Dijkstra: tra ve chi phi nho nhat start -> target.
// parent[v] luu node dung ngay truoc v, de in duong di.
int dijkstra(int n, int start, int target, int parent[]) {
    int distance[MAX_NODE];
    bool visited[MAX_NODE];

    for (int i = 0; i < n; i++) {
        distance[i] = INF;
        visited[i] = false;
        parent[i] = -1;
    }
    distance[start] = 0;

    for (int count = 0; count < n; count++) {
        int u = minDistance(distance, visited, n);
        if (u == -1) break; // Cac node con lai khong the toi duoc.
        visited[u] = true;

        for (int v = 0; v < n; v++) {
            // Chi dung canh co ton tai va chua bi Defender block.
            if (!visited[v] && cost[u][v] != INF && !blocked[u][v]) {
                int newCost = distance[u] + cost[u][v];
                if (newCost < distance[v]) {
                    distance[v] = newCost;
                    parent[v] = u;
                }
            }
        }
    }
    return distance[target];
}

// In duong di theo chieu start -> target.
void printPath(int parent[], int start, int target) {
    int path[MAX_NODE];
    int length = 0;

    for (int current = target; current != -1; current = parent[current]) {
        path[length] = current;
        length++;
    }

    // Duong hop le phai ket thuc o start.
    if (path[length - 1] != start) return;

    for (int i = length - 1; i >= 0; i--) {
        cout << path[i];
        if (i > 0) cout << " -> ";
    }
}

// Ham ma P1 goi truoc va sau patch.
void hackerSimulation(int n, int start, int target, int budget) {
    int parent[MAX_NODE];
    int minCost = dijkstra(n, start, target, parent);

    cout << "Source: " << start << " | Target: " << target << '\n';
    cout << "Budget: " << budget << " token\n";

    if (minCost == INF) {
        cout << "Hacker FAIL: khong con duong den Target.\n";
    } else {
        cout << "Duong re nhat: ";
        printPath(parent, start, target);
        cout << "\nChi phi: " << minCost << " token\n";

        if (minCost <= budget) {
            cout << "Hacker SUCCESS. Token con lai: "
                 << budget - minCost << "\n";
        } else {
            cout << "Hacker FAIL: chi phi vuot Budget.\n";
        }
    }
}

void initializeGraph(int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            cost[i][j] = INF;
            blocked[i][j] = false;
        }
    }
}

int main() {
    // Demo 4 node: 0=Entry, 1=PC, 2=Admin, 3=Target.
    int n = 4;
    int start = 0;
    int target = 3;
    int budget = 15;
    initializeGraph(n);

    cost[0][1] = 4;
    cost[1][2] = 5;
    cost[2][3] = 4;   // Path cu: 0 -> 1 -> 2 -> 3, cost 13.
    cost[1][3] = 15;  // Path thay the: 0 -> 1 -> 3, cost 19.

    cout << "=== BEFORE PATCH ===\n";
    hackerSimulation(n, start, target, budget);

    // Dau noi voi P3: P3 tim Min-Cut; P1 dat blocked[u][v] = true.
    blocked[2][3] = true;

    cout << "\n=== AFTER PATCH: block edge 2 -> 3 ===\n";
    hackerSimulation(n, start, target, budget);
    return 0;
}
