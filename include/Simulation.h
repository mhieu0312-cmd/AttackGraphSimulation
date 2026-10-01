#pragma once
#include "Defender.h"
#include "Dijkstra.h"

struct SimulationResult {
    AttackResult before, after;
    bool reachableBefore = false, reachableAfter = false;
    std::vector<int> patchedEdgeIds;
    std::string defenseMessage;
};
// Khong co Max-Flow/Min-Cut: tu dong dung suggestCutEdge cua prototype P3.
// manualPatch chi dung khi nguoi goi chu dong truyen danh sach ID.
SimulationResult runSimulation(Graph& graph, int source, int target, std::int64_t budget,
    std::optional<std::vector<int>> manualPatch = std::nullopt);
