#pragma once
#ifndef DEFENDER_H
#define DEFENDER_H

#include <vector>
#include <iostream>
#include <queue>

// Cấu trúc một cạnh (quyền truy cập giữa 2 máy/tài khoản)
struct Edge {
    int id;          // ID cạnh
    int u, v;        // Nối từ node u -> node v
    bool blocked;    // Trạng thái: false = bình thường, true = đã bị Blue Team cắt
};

class Defender {
private:
    int numNodes;
    std::vector<Edge> edges;
    std::vector<std::vector<int>> adj; // Danh sách kề chứa index của edges

public:
    Defender(int nodes);
    void addEdge(int id, int u, int v);

    // 1. Thuật toán kiểm tra kết nối (Connectivity)
    bool isReachable(int source, int target);

    // 2. Thuật toán đề xuất cắt cạnh (Min-Cut đơn giản)
    int suggestCutEdge(int source, int target);

    // 3. Thực hiện vá lỗi (Patch)
    void blockEdge(int edgeId);
};

#endif
