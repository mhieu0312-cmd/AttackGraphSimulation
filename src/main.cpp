#include "LoadDataset.h"
#include "Simulation.h"
#include "RuntimeInput.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

namespace {

// Huong dan cach chay
void usage() {

    cout
        << "Usage:\n"
        << "  AttackGraph\n"
        << "  AttackGraph --demo [dataset.json] (first ENTRY/TARGET, budget 0)\n"
        << "  AttackGraph <dataset.json> (interactive)\n"
        << "  AttackGraph <dataset> <source> <target> <budget>\n"
        << "  AttackGraph <dataset> <source> <target> <budget> "
           "--patch <edgeId...>\n";
}

}

int main(int argc, char* argv[]) {

    try {

        // =========================
        // CAU HINH MAC DINH
        // =========================

        string filename;

        SimulationParameters parameters{0, 0, 0};
        optional<vector<int>> manual;
        bool interactive = true;
        bool demo = false;
        if (argc == 2 && string(argv[1]) == "--help") {
            usage();
            return 0;
        }
        if (argc >= 2 && string(argv[1]) == "--demo") {
            if (argc > 3) throw invalid_argument("Usage: AttackGraph --demo [dataset.json]");
            demo = true;
            interactive = false;
            if (argc == 3) filename = argv[2];
        } else if (argc == 2) {
            filename = argv[1];
        } else if (argc > 1) {
            if (argc < 5) { usage(); return 1; }
            filename = argv[1];
            parameters = {parseNodeId(argv[2]), parseNodeId(argv[3]), parseNonnegative(argv[4])};
            interactive = false;
            if (argc > 5) {
                if (string(argv[5]) != "--patch" || argc == 6)
                    throw invalid_argument("Can --patch va it nhat mot edge ID");
                manual = vector<int>{};
                for (int i=6; i<argc; ++i) manual->push_back(parseNodeId(argv[i]));
            }
        }

        // =========================
        // LOAD DATASET
        // =========================

        filename = resolveDatasetPath(filename, argv[0], PROJECT_ROOT).string();
        cout << "File: " << filename << '\n';
        Graph graph = loadDataset(filename);
        for (const auto& warning : graph.validationWarnings()) cout << "Warning: " << warning << '\n';
        if (interactive) parameters = selectParameters(graph, cin, cout);
        else if (demo) parameters = demoParameters(graph);
        validateParameters(graph, parameters);
        const int source = parameters.source;
        const int target = parameters.target;
        const int64_t budget = parameters.budget;

        auto stats = graph.statistics();

        cout << "\n=== DATASET ===\n";
        cout << "Nodes: " << stats.nodes << '\n';
        cout << "Edges: " << stats.edges << '\n';
        cout << "Source: " << source << '\n';
        cout << "Target: " << target << '\n';
        cout << "Budget: " << budget << '\n';


        // =========================
        // CHAY MO PHONG
        // =========================

        auto result = runSimulation(
            graph,
            source,
            target,
            budget,
            manual
        );


        // =========================
        // TRUOC KHI PATCH
        // =========================

        cout << "\n=== BEFORE PATCH ===\n";

        printAttack(
            result.before,
            cout
        );


        // =========================
        // PHONG THU
        // =========================

        cout << "\n=== DEFENSE ===\n";

        cout
            << result.defenseMessage
            << '\n';

        if (result.minCut) {
            cout << "Max-Flow: " << result.minCut->maxFlow << '\n';
            cout << "Min-Cut Capacity: " << result.minCut->minCutCapacity << '\n';
        } else {
            cout << "Max-Flow: N/A (Min-Cut khong duoc tinh)\n";
            cout << "Min-Cut Capacity: N/A (Min-Cut khong duoc tinh)\n";
        }
        cout << "Patched edges: ";

        if (result.patchedEdgeIds.empty()) {

            cout << "none";
        }
        else {

            for (int id :
                 result.patchedEdgeIds) {

                cout << id << ' ';
            }
        }

        cout << '\n';
        cout << "Newly blocked edges: " << result.newlyBlockedEdges << '\n';


        // =========================
        // SAU KHI PATCH
        // =========================

        cout << "\n=== AFTER PATCH ===\n";

        printAttack(
            result.after,
            cout
        );


        // =========================
        // KET QUA
        // =========================

        cout << "\n=== RESULT ===\n";

        cout
            << "Reachable: "
            << result.reachableBefore
            << " -> "
            << result.reachableAfter
            << '\n';

        cout
            << "Blocked edges: "
            << stats.blockedEdges
            << " -> "
            << graph.statistics().blockedEdges
            << '\n';

        return 0;
    }


    // =========================
    // BAT LOI
    // =========================

    catch (const exception& e) {

        cerr
            << "ERROR: "
            << e.what()
            << '\n';

        return 1;
    }
}