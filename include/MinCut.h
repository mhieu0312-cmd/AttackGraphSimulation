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

// Edmonds-Karp dung Capacity, tra ve ID canh goc can Patch.
// Khong thay doi graph; kiem tra tap cut co ngat S-T hay khong.
MinCutResult minimumSTCut(const Graph& graph, int source, int target);
