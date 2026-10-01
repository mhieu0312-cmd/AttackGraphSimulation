#include "Simulation.h"

SimulationResult runSimulation(Graph& graph, int source, int target, std::int64_t budget,
                                std::optional<std::vector<int>> manualPatch) {
    SimulationResult result;
    Defender defender(graph);
    result.before = hackerSimulation(graph, source, target, budget);
    result.reachableBefore = defender.isReachable(source, target);
    if (manualPatch) {
        result.patchedEdgeIds = *manualPatch;
        result.defenseMessage = "Patch thu cong theo edge ID; khong phai ket qua Min-Cut.";
    } else if (!result.reachableBefore) {
        result.defenseMessage = "S-T da mat ket noi; khong can patch.";
    } else if (source == target) {
        result.defenseMessage = "S == T: duong rong cost 0, chan canh khong tach duoc S khoi chinh no.";
    } else {
        int id = defender.suggestCutEdge(source, target);
        if (id >= 0) {
            result.patchedEdgeIds.push_back(id);
            result.defenseMessage = "Prototype P3 tim thay mot canh don le ngat S-T. Chua toi uu capacity.";
        } else result.defenseMessage = "Khong co canh don le ngat S-T. Can Max-Flow/Min-Cut tu Nguoi 3.";
    }
    graph.blockEdges(result.patchedEdgeIds);
    // Cung graph da patch, cung budget ban dau; khong dung remainingToken cua lan truoc.
    result.after = hackerSimulation(graph, source, target, budget);
    result.reachableAfter = defender.isReachable(source, target);
    return result;
}
