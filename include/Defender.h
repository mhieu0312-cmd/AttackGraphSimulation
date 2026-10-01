#pragma once
#include "Graph.h"
#include <vector>

// Cấu trúc lưu kết quả Min-Cut của Blue Team
struct CutResult {
    std::vector<int> cutEdgeIds;   // Danh sách ID các cạnh cần vá (Patch)
    int totalDefenderCost = 0;     // Tổng chi phí tác động vận hành \sum c_{def}
};

class Defender {
    const Graph& graph;
public:
    explicit Defender(const Graph& graph) : graph(graph) {}
    
    // Kiểm tra tính liên thông S-T
    bool isReachable(int source, int target) const;
    
    // Thuật toán Min-Cut tối ưu dựa trên trọng số c_def (capacity)
    CutResult findMinCut(int source, int target) const;
    
    void blockEdge(int edgeId);
};
