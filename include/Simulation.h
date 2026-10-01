#pragma once

#include "Defender.h"
#include "Dijkstra.h"

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
};


// =========================
// CHAY MO PHONG
// =========================

// Chay Attack -> Defense -> Attack lai
SimulationResult runSimulation(
    Graph& graph,
    int source,
    int target,
    int64_t budget,
    optional<vector<int>> manualPatch = nullopt
);