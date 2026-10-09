#include "Simulation.h"

using namespace std;


// =========================
// CHAY MO PHONG
// =========================

SimulationResult runSimulation(
    Graph& graph,
    int source,
    int target,
    int64_t budget,
    optional<vector<int>> manualPatch
) {

    SimulationResult result;

    Defender defender(graph);


    // =========================
    // TRUOC KHI PHONG THU
    // =========================

    // Hacker tan cong lan dau
    result.before =
        hackerSimulation(
            graph,
            source,
            target,
            budget
        );

    // Kiem tra S -> T con ket noi khong
    result.reachableBefore =
        defender.isReachable(
            source,
            target
        );


    // =========================
    // CHON EDGE CAN PATCH
    // =========================

    // Patch thu cong
    if (manualPatch) {

        result.patchedEdgeIds =
            *manualPatch;

        result.defenseMessage =
            "Patch thu cong theo edge ID.";
    }

    // S va T da mat ket noi
    else if (!result.reachableBefore) {

        result.defenseMessage =
            "S-T da mat ket noi, khong can patch.";
    }

    // Source va target la cung mot node
    else if (source == target) {

        result.defenseMessage =
            "Source va target la cung mot node.";
    }

    // Tu dong tim edge can block
    else {

        int edgeId =
            defender.suggestCutEdge(
                source,
                target
            );

        if (edgeId >= 0) {

            result.patchedEdgeIds.push_back(
                edgeId
            );

            result.defenseMessage =
                "Tim thay edge co the ngat ket noi S-T.";
        }
        else {

            result.defenseMessage =
                "Khong co mot edge don le nao ngat duoc S-T.";
        }
    }


    // =========================
    // PATCH EDGE
    // =========================

    graph.blockEdges(
        result.patchedEdgeIds
    );


    // =========================
    // SAU KHI PHONG THU
    // =========================

    // Hacker tan cong lai voi budget ban dau
    result.after =
        hackerSimulation(
            graph,
            source,
            target,
            budget
        );

    // Kiem tra lai ket noi
    result.reachableAfter =
        defender.isReachable(
            source,
            target
        );


    return result;
}