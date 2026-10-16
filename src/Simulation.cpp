#include "Simulation.h"

#include <stdexcept>
#include <unordered_set>

SimulationResult runSimulation(Graph& graph, int source, int target, int64_t budget,
                               optional<vector<int>> manualPatch) {
    SimulationResult result;
    Defender defender(graph);

    if (manualPatch) {
        for (int id : *manualPatch) {
            if (!graph.getEdge(id)) {
                throw invalid_argument("Patch: khong co edge ID " + to_string(id));
            }
        }
    }

    result.before = hackerSimulation(graph, source, target, budget);
    result.reachableBefore = defender.isReachable(source, target);

    if (!result.reachableBefore) {
        result.defenseMessage = "S-T da mat ket noi, khong can patch.";
    } else if (source == target) {
        result.defenseMessage =
            "Source va target la cung mot node; khong tinh Min-Cut va khong patch.";
    } else if (manualPatch) {
        // bo id trung va canh da chan
        unordered_set<int> selected;
        for (int id : *manualPatch) {
            if (!graph.isBlocked(id) && selected.count(id) == 0) {
                selected.insert(id);
                result.patchedEdgeIds.push_back(id);
            }
        }
        result.defenseMessage = "Patch thu cong theo edge ID.";
    } else {
        result.minCut = minimumSTCut(graph, source, target);
        result.patchedEdgeIds = result.minCut->edgeIds;
        result.defenseMessage = "Auto Patch toan bo tap Minimum S-T Cut theo capacity.";
    }

    for (int id : result.patchedEdgeIds) {
        if (!graph.getEdge(id)) {
            throw invalid_argument("Patch: khong co edge ID " + to_string(id));
        }
    }
    graph.blockEdges(result.patchedEdgeIds);
    result.newlyBlockedEdges = result.patchedEdgeIds.size();

    // cap lai budget ban dau
    result.after = hackerSimulation(graph, source, target, budget, &result.before.path);
    result.reachableAfter = defender.isReachable(source, target);
    if (result.minCut && result.reachableAfter) {
        throw logic_error("Auto Patch: tap Min-Cut khong ngat duoc S-T");
    }
    return result;
}
