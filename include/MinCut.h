#pragma once

#include "Graph.h"
#include <cstdint>
#include <vector>

using namespace std;

struct MinCutResult {
    int64_t maxFlow = 0;
    int64_t minCutCapacity = 0;
    vector<int> edgeIds;
};

// tim cut theo capacity, tra id canh goc

MinCutResult minimumSTCut(const Graph& graph, int source, int target);
