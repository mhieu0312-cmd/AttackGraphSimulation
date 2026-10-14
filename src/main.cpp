#include "LoadDataset.h"
#include "Simulation.h"

#include <filesystem>
#include <iostream>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

namespace {
// Tham so cho mot lan mo phong.
struct SimulationParameters {
    int source;
    int target;
    int64_t budget;
};
namespace fs = filesystem;

fs::path resolveDatasetPath(const fs::path& explicitPath, const fs::path& executable,
                            const fs::path& projectRoot) {
    if (!explicitPath.empty()) {
        return fs::absolute(explicitPath).lexically_normal();
    }
    const vector<fs::path> candidates{projectRoot / "data/graph.json",
                                      fs::absolute(executable).parent_path() / "data/graph.json",
                                      fs::absolute(executable).parent_path() / "graph.json",
                                      fs::current_path() / "data/graph.json"};
    for (const fs::path& path : candidates) {
        if (fs::is_regular_file(path)) {
            return fs::absolute(path).lexically_normal();
        }
    }
    throw runtime_error("Khong tim thay dataset. Truyen duong dan JSON hoac dat data/graph.json "
                        "canh chuong trinh.");
}

int64_t parseNonnegative(const string& text) {
    if (text.empty() || text.find_first_not_of("0123456789") != string::npos) {
        throw invalid_argument("Gia tri phai la so nguyen khong am");
    }
    try {
        return stoll(text);
    } catch (const out_of_range&) {
        throw invalid_argument("Gia tri vuot mien int64_t");
    }
}

int parseNodeId(const string& text) {
    int64_t value = parseNonnegative(text);
    if (value > numeric_limits<int>::max()) {
        throw invalid_argument("ID vuot mien int");
    }
    return static_cast<int>(value);
}

void validateParameters(const Graph& graph, const SimulationParameters& p) {
    if (!graph.getNode(p.source)) {
        throw invalid_argument("Source ID khong ton tai: " + to_string(p.source));
    }
    if (!graph.getNode(p.target)) {
        throw invalid_argument("Target ID khong ton tai: " + to_string(p.target));
    }
    if (p.budget < 0) {
        throw invalid_argument("Budget phai >= 0");
    }
}

string readLine(istream& input) {
    string line;
    if (!getline(input, line)) {
        throw runtime_error("Ket thuc input; can nhap Source, Target va Budget hoac dung CLI.");
    }
    size_t first = line.find_first_not_of(" \t\r\n");
    if (first == string::npos) {
        return {};
    }
    return line.substr(first, line.find_last_not_of(" \t\r\n") - first + 1);
}

int chooseNode(const Graph& graph, const char* label, istream& in, ostream& out) {
    while (true) {
        out << label << " ID (co the chon bat ky node, ke ca ENDPOINT): " << flush;
        string line = readLine(in);
        try {
            int id = parseNodeId(line);
            if (!graph.getNode(id)) {
                throw invalid_argument("Node ID khong ton tai");
            }
            return id;
        } catch (const invalid_argument& e) {
            out << "ERROR: " << e.what() << '\n';
        }
    }
}

SimulationParameters selectParameters(const Graph& graph, istream& in, ostream& out) {
    if (graph.getNodes().empty()) {
        throw invalid_argument("Graph rong; khong co node de chon");
    }
    for (NodeType type : {NodeType::ENTRY, NodeType::TARGET}) {
        out << nodeTypeToString(type) << ":\n";
        bool found = false;
        for (const Node& node : graph.getNodes()) {
            if (node.getType() == type) {
                out << "  " << node.getID() << " - " << node.getName() << '\n';
                found = true;
            }
        }
        if (!found) {
            out << "  Khong co " << nodeTypeToString(type) << "; chon node khac tu danh sach.\n";
        }
    }
    out << "Tat ca node (ID - name - type):\n";
    for (const Node& node : graph.getNodes()) {
        out << "  " << node.getID() << " - " << node.getName() << " - "
            << nodeTypeToString(node.getType()) << '\n';
    }
    SimulationParameters p{chooseNode(graph, "Source", in, out),
                           chooseNode(graph, "Target", in, out), 0};
    while (true) {
        out << "Token Budget: " << flush;
        string line = readLine(in);
        try {
            p.budget = parseNonnegative(line);
            break;
        } catch (const invalid_argument& e) {
            out << "ERROR: " << e.what() << '\n';
        }
    }
    validateParameters(graph, p);
    return p;
}

int firstNodeOfType(const Graph& graph, NodeType type) {
    for (const Node& node : graph.getNodes()) {
        if (node.getType() == type) {
            return node.getID();
        }
    }
    throw invalid_argument("Demo can node " + nodeTypeToString(type) +
                           "; dung che do tuong tac/CLI de chon node khac.");
}

SimulationParameters demoParameters(const Graph& graph) {
    return {firstNodeOfType(graph, NodeType::ENTRY), firstNodeOfType(graph, NodeType::TARGET), 0};
}

// Huong dan cach chay
void usage() {
    cout << "Usage:\n"
         << "  AttackGraph\n"
         << "  AttackGraph --demo [dataset.json] (first ENTRY/TARGET, budget 0)\n"
         << "  AttackGraph <dataset.json> (interactive)\n"
         << "  AttackGraph <dataset> <source> <target> <budget>\n"
         << "  AttackGraph <dataset> <source> <target> <budget> "
            "--patch <edgeId...>\n";
}

} // namespace

int main(int argc, char* argv[]) {
    try {
        // CAU HINH MAC DINH

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
            if (argc > 3) {
                throw invalid_argument("Usage: AttackGraph --demo [dataset.json]");
            }
            demo = true;
            interactive = false;
            if (argc == 3) {
                filename = argv[2];
            }
        } else if (argc == 2) {
            filename = argv[1];
        } else if (argc > 1) {
            if (argc < 5) {
                usage();
                return 1;
            }
            filename = argv[1];
            parameters = {parseNodeId(argv[2]), parseNodeId(argv[3]), parseNonnegative(argv[4])};
            interactive = false;
            if (argc > 5) {
                if (string(argv[5]) != "--patch" || argc == 6) {
                    throw invalid_argument("Can --patch va it nhat mot edge ID");
                }
                manual = vector<int>{};
                for (int i = 6; i < argc; ++i) {
                    manual->push_back(parseNodeId(argv[i]));
                }
            }
        }

        // LOAD DATASET

        filename = resolveDatasetPath(filename, argv[0], PROJECT_ROOT).string();
        cout << "File: " << filename << '\n';
        Graph graph = loadDataset(filename);
        for (const string& warning : graph.validationWarnings()) {
            cout << "Warning: " << warning << '\n';
        }
        if (interactive) {
            parameters = selectParameters(graph, cin, cout);
        } else if (demo) {
            parameters = demoParameters(graph);
        }
        validateParameters(graph, parameters);
        const int source = parameters.source;
        const int target = parameters.target;
        const int64_t budget = parameters.budget;
        GraphStatistics stats = graph.statistics();
        cout << "\n=== DATASET ===\n";
        cout << "Nodes: " << stats.nodes << '\n';
        cout << "Edges: " << stats.edges << '\n';
        cout << "Source: " << source << '\n';
        cout << "Target: " << target << '\n';
        cout << "Budget: " << budget << '\n';

        // CHAY MO PHONG

        SimulationResult result = runSimulation(graph, source, target, budget, manual);

        // TRUOC KHI PATCH

        cout << "\n=== BEFORE PATCH ===\n";
        printAttack(result.before, cout);

        // PHONG THU

        cout << "\n=== DEFENSE ===\n";
        cout << result.defenseMessage << '\n';

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
        } else {
            for (int id : result.patchedEdgeIds) {
                cout << id << ' ';
            }
        }

        cout << '\n';
        cout << "Newly blocked edges: " << result.newlyBlockedEdges << '\n';

        // SAU KHI PATCH

        cout << "\n=== AFTER PATCH ===\n";
        printAttack(result.after, cout);

        // KET QUA

        cout << "\n=== RESULT ===\n";
        cout << "Reachable: " << result.reachableBefore << " -> " << result.reachableAfter << '\n';

        cout << "Blocked edges: " << stats.blockedEdges << " -> " << graph.statistics().blockedEdges
             << '\n';
        return 0;
    }

    // BAT LOI

    catch (const exception& e) {
        cerr << "ERROR: " << e.what() << '\n';
        return 1;
    }
}
