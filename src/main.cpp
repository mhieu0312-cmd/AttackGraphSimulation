#include "LoadDataset.h"
#include "Simulation.h"

#include <algorithm>
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

// Input configuration is separate from the simulation algorithms.
struct SimulationParameters {
    int source;
    int target;
    std::int64_t budget;
};

namespace fs = std::filesystem;
fs::path resolveDatasetPath(const fs::path& explicitPath, const fs::path& executable,
                            const fs::path& projectRoot) {
    if (!explicitPath.empty()) return fs::absolute(explicitPath).lexically_normal();
    const std::vector<fs::path> candidates{
        projectRoot / "data/graph.json",
        fs::absolute(executable).parent_path() / "data/graph.json",
        fs::absolute(executable).parent_path() / "graph.json",
        fs::current_path() / "data/graph.json"
    };
    for (const auto& path : candidates) {
        if (fs::is_regular_file(path)) return fs::absolute(path).lexically_normal();
    }
    throw std::runtime_error("Khong tim thay dataset. Truyen duong dan JSON hoac dat data/graph.json canh chuong trinh.");
}
std::int64_t parseNonnegative(const std::string& text) {
    if (text.empty() || text.find_first_not_of("0123456789") != std::string::npos)
        throw std::invalid_argument("Gia tri phai la so nguyen khong am");
    try { return std::stoll(text); }
    catch (const std::out_of_range&) { throw std::invalid_argument("Gia tri vuot mien int64_t"); }
}
int parseNodeId(const std::string& text) {
    const auto value = parseNonnegative(text);
    if (value > std::numeric_limits<int>::max()) throw std::invalid_argument("ID vuot mien int");
    return static_cast<int>(value);
}
void validateParameters(const Graph& graph, const SimulationParameters& p) {
    if (!graph.getNode(p.source)) throw std::invalid_argument("Source ID khong ton tai: " + std::to_string(p.source));
    if (!graph.getNode(p.target)) throw std::invalid_argument("Target ID khong ton tai: " + std::to_string(p.target));
    if (p.budget < 0) throw std::invalid_argument("Budget phai >= 0");
}
namespace {
std::string readLine(std::istream& input) {
    std::string line;
    if (!std::getline(input, line)) throw std::runtime_error("Ket thuc input; can nhap Source, Target va Budget hoac dung CLI.");
    const auto first = line.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    return line.substr(first, line.find_last_not_of(" \t\r\n") - first + 1);
}
int chooseNode(const Graph& graph, const char* label, std::istream& in, std::ostream& out) {
    while (true) {
        out << label << " ID (co the chon bat ky node, ke ca ENDPOINT): " << std::flush;
        const auto line = readLine(in);
        try {
            const auto id = parseNodeId(line);
            if (!graph.getNode(id)) throw std::invalid_argument("Node ID khong ton tai");
            return id;
        } catch (const std::invalid_argument& e) { out << "ERROR: " << e.what() << '\n'; }
    }
}
}
SimulationParameters selectParameters(const Graph& graph, std::istream& in, std::ostream& out) {
    if (graph.getNodes().empty()) throw std::invalid_argument("Graph rong; khong co node de chon");
    for (const auto type : {NodeType::ENTRY, NodeType::TARGET}) {
        out << nodeTypeToString(type) << ":\n";
        bool found = false;
        for (const auto& node : graph.getNodes()) {
            if (node.getType() == type) { out << "  " << node.getID() << " - " << node.getName() << '\n'; found = true; }
        }
        if (!found) out << "  Khong co " << nodeTypeToString(type) << "; chon node khac tu danh sach.\n";
    }
    out << "Tat ca node (ID - name - type):\n";
    for (const auto& node : graph.getNodes())
        out << "  " << node.getID() << " - " << node.getName() << " - " << nodeTypeToString(node.getType()) << '\n';
    SimulationParameters p{chooseNode(graph,"Source",in,out), chooseNode(graph,"Target",in,out),0};
    while (true) {
        out << "Token Budget: " << std::flush;
        const auto line = readLine(in);
        try { p.budget = parseNonnegative(line); break; }
        catch (const std::invalid_argument& e) { out << "ERROR: " << e.what() << '\n'; }
    }
    validateParameters(graph,p);
    return p;
}
SimulationParameters demoParameters(const Graph& graph) {
    auto first = [&](NodeType type) {
        for (const auto& node : graph.getNodes()) if (node.getType()==type) return node.getID();
        throw std::invalid_argument("Demo can node " + nodeTypeToString(type) + "; dung che do tuong tac/CLI de chon node khac.");
    };
    return {first(NodeType::ENTRY), first(NodeType::TARGET), 0};
}

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