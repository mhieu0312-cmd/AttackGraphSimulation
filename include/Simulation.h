#pragma once

#include "Defender.h"
#include "Dijkstra.h"
#include "MinCut.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

using namespace std;

struct SimulationResult {

    AttackResult before;
    AttackResult after;

    bool reachableBefore = false;
    bool reachableAfter = false;

    vector<int> patchedEdgeIds;

    string defenseMessage;

    // chi co khi da tinh min-cut
    optional<MinCutResult> minCut;
    size_t newlyBlockedEdges = 0;
};

// moi ca doc lap can nap lai graph
SimulationResult runSimulation(Graph& graph, int source, int target, int64_t budget,
                               optional<vector<int>> manualPatch = nullopt);
