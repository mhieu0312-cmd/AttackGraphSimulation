#include "LoadDataset.h"
#include "Simulation.h"

#include <iostream>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

std::int64_t number(const char* text) {
    std::string s(text);

    if (s.empty() || s.find_first_not_of("0123456789") != std::string::npos) {
        throw std::invalid_argument("Tham so phai la so nguyen khong am");
    }

    std::size_t end = 0;
    std::int64_t value = std::stoll(s, &end);

    if (end != s.size()) {
        throw std::invalid_argument("Tham so khong hop le");
    }

    return value;
}

int idNumber(const char* text) {
    std::int64_t value = number(text);

    if (value > std::numeric_limits<int>::max()) {
        throw std::invalid_argument("ID vuot mien int");
    }

    return static_cast<int>(value);
}

void usage() {
    std::cout
        << "Usage:\n"
        << "  AttackGraph\n"
        << "  AttackGraph --demo\n"
        << "  AttackGraph <dataset> <source> <target> <budget>\n"
        << "  AttackGraph <dataset> <source> <target> <budget> --patch <edgeId...>\n";
}

}


int main(int argc, char* argv[]) {
    try {

        // =========================
        // DEFAULT CONFIGURATION
        // =========================

        std::string filename =
            std::string(PROJECT_ROOT) + "/data/graph.json";

        int source = 0;
        int target = 37;
        std::int64_t budget = 15;

        std::optional<std::vector<int>> manual;


        // =========================
        // HELP
        // =========================

        if (argc == 2 && std::string(argv[1]) == "--help") {
            usage();
            return 0;
        }


        // =========================
        // DEMO DATASET
        // =========================

        if (argc == 2 && std::string(argv[1]) == "--demo") {

            filename =
                std::string(PROJECT_ROOT)
                + "/tests/fixtures/p2_demo.json";

            source = 0;
            target = 3;
            budget = 15;

            std::cout << "Running demo dataset\n";
        }


        // =========================
        // MANUAL ARGUMENTS
        // =========================

        else if (argc > 1) {

            if (argc < 5) {
                usage();
                return 1;
            }

            filename = argv[1];
            source = idNumber(argv[2]);
            target = idNumber(argv[3]);
            budget = number(argv[4]);


            // Manual patch
            if (argc > 5) {

                if (std::string(argv[5]) != "--patch" || argc == 6) {
                    throw std::invalid_argument(
                        "Can --patch va it nhat mot edge ID"
                    );
                }

                manual = std::vector<int>{};

                for (int i = 6; i < argc; ++i) {
                    manual->push_back(idNumber(argv[i]));
                }
            }
        }


        // =========================
        // LOAD DATASET
        // =========================

        Graph graph = loadDataset(filename);

        auto stats = graph.statistics();

        std::cout << "\n=== DATASET ===\n";
        std::cout << "File: " << filename << '\n';
        std::cout << "Nodes: " << stats.nodes << '\n';
        std::cout << "Edges: " << stats.edges << '\n';
        std::cout << "Source: " << source << '\n';
        std::cout << "Target: " << target << '\n';
        std::cout << "Budget: " << budget << '\n';


        // Validation warnings
        for (const auto& warning : graph.validationWarnings()) {
            std::cout << "Warning: " << warning << '\n';
        }


        // =========================
        // SIMULATION
        // =========================

        auto result =
            runSimulation(graph, source, target, budget, manual);


        std::cout << "\n=== BEFORE PATCH ===\n";
        printAttack(result.before, std::cout);


        std::cout << "\n=== DEFENSE ===\n";
        std::cout << result.defenseMessage << '\n';

        std::cout << "Patched edges: ";

        if (result.patchedEdgeIds.empty()) {
            std::cout << "none";
        }
        else {
            for (int id : result.patchedEdgeIds) {
                std::cout << id << ' ';
            }
        }

        std::cout << '\n';


        std::cout << "\n=== AFTER PATCH ===\n";
        printAttack(result.after, std::cout);


        // =========================
        // RESULT
        // =========================

        std::cout << "\n=== RESULT ===\n";

        std::cout
            << "Reachable: "
            << result.reachableBefore
            << " -> "
            << result.reachableAfter
            << '\n';

        std::cout
            << "Blocked edges: "
            << stats.blockedEdges
            << " -> "
            << graph.statistics().blockedEdges
            << '\n';


        return 0;
    }

    catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << '\n';
        return 1;
    }
}