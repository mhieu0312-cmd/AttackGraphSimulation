#pragma once

#include "Defender.h"
#include "Dijkstra.h"
#include "MinCut.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

using namespace std;


// =========================
// KET QUA MO PHONG
// =========================

struct SimulationResult {

    // Ket qua hacker truoc va sau khi phong thu
    AttackResult before;
    AttackResult after;

    // Trang thai ket noi S -> T
    bool reachableBefore = false;
    bool reachableAfter = false;

    // Danh sach edge da bi Defender block
    vector<int> patchedEdgeIds;

    // Thong bao ket qua phong thu
    string defenseMessage;

    // Chi co gia tri khi Auto Defense da tinh Min-Cut.
    optional<MinCutResult> minCut;
    size_t newlyBlockedEdges = 0;
};


// =========================
// CHAY MO PHONG
// =========================

// Chay Attack -> Defense -> Attack lai. Graph truyen vao giu cac canh da patch.
// Moi scenario doc lap phai copy graph ban dau hoac load lai dataset.
SimulationResult runSimulation(
    Graph& graph,
    int source,
    int target,
    int64_t budget,
    optional<vector<int>> manualPatch = nullopt
);