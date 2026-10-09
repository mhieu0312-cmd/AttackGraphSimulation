#pragma once

#include "Graph.h"
#include <cstdint>
#include <vector>

struct MinCutResult {
    std::int64_t maxFlow = 0;
    std::int64_t minCutCapacity = 0;
    std::vector<int> edgeIds;
};

// Edmonds-Karp on active directed edges, using capacity only.
// Returns original edge IDs (including zero-capacity cut edges).
// Does not mutate graph. Throws invalid_argument for absent/equal endpoints,
// overflow_error for int64_t overflow, logic_error if cut verification fails.
MinCutResult minimumSTCut(const Graph& graph, int source, int target);
