#include "LoadDataset.h"
#include "Simulation.h"
#include <filesystem>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace {
std::int64_t number(const char* text) {
    std::string s(text);
    if (s.empty() || s.find_first_not_of("0123456789") != std::string::npos)
        throw std::invalid_argument("Tham so phai la so nguyen khong am");
    std::size_t end = 0;
    auto value = std::stoll(s, &end);
    if (end != s.size()) throw std::invalid_argument("Tham so khong hop le");
    return value;
}
int idNumber(const char* text) {
    auto value = number(text);
    if (value > std::numeric_limits<int>::max()) throw std::invalid_argument("ID vuot mien int");
    return static_cast<int>(value);
}
void usage() {
    std::cout << "AttackGraph [--demo]\n"
                 "AttackGraph <dataset.json> <source> <target> <budget> [--patch <edgeId> ...]\n"
                 "Mac dinh: graph mau 4 node cua P2. KHONG phai dataset that.\n"
                 "Tu dong: prototype single-edge cut cua P3, CHUA co weighted Min-Cut.\n";
}
}
int main(int argc, char* argv[]) {
    try {
        if (argc == 2 && std::string(argv[1]) == "--help") { usage(); return 0; }
        bool demo = argc == 1 || (argc == 2 && std::string(argv[1]) == "--demo");
        std::string filename;
        int source = 0, target = 3;
        std::int64_t budget = 15;
        std::optional<std::vector<int>> manual;
        if (demo) {
            filename = (std::filesystem::absolute(argv[0]).parent_path() / "fixtures/p2_demo.json").string();
            std::cout << "DEMO FIXTURE: 4 node tu prototype P2, KHONG phai dataset that.\n";
        } else {
            if (argc < 5) { usage(); return 1; }
            filename = argv[1]; source = idNumber(argv[2]); target = idNumber(argv[3]); budget = number(argv[4]);
            if (argc > 5) {
                if (std::string(argv[5]) != "--patch" || argc == 6)
                    throw std::invalid_argument("Can --patch va it nhat mot edge ID");
                manual = std::vector<int>{};
                for (int i = 6; i < argc; ++i) manual->push_back(idNumber(argv[i]));
            }
        }
        Graph graph = loadDataset(filename);
        auto stats = graph.statistics();
        std::cout << "Dataset: " << filename << "\nNodes=" << stats.nodes << " Edges=" << stats.edges
                  << " Active=" << stats.activeEdges << " Blocked=" << stats.blockedEdges << '\n';
        for (const auto& [type, count] : stats.nodeTypes) std::cout << "Type " << type << ": " << count << '\n';
        for (const auto& [relation, count] : stats.relations) std::cout << "Relation " << relation << ": " << count << '\n';
        for (const auto& [id, degree] : stats.inDegree)
            std::cout << "Node " << id << ": in=" << degree << " out=" << stats.outDegree.at(id) << '\n';
        for (const auto& warning : graph.validationWarnings()) std::cout << "NOTE: " << warning << '\n';
        std::cout << "S=" << source << " T=" << target << '\n';
        auto result = runSimulation(graph, source, target, budget, manual);
        std::cout << "\n=== BEFORE PATCH ===\n";
        printAttack(result.before, std::cout);
        std::cout << "\n=== DEFENDER / PATCH ===\n" << result.defenseMessage << "\nPatched IDs:";
        for (int id : result.patchedEdgeIds) std::cout << ' ' << id;
        if (result.patchedEdgeIds.empty()) std::cout << " (none)";
        std::cout << "\n\n=== AFTER PATCH ===\n";
        printAttack(result.after, std::cout);
        std::cout << "\nReachable: " << result.reachableBefore << " -> " << result.reachableAfter
                  << "\nBlocked edges: " << stats.blockedEdges << " -> " << graph.statistics().blockedEdges
                  << "\nEdges van giu nguyen: " << graph.getEdges().size()
                  << "\nLIMIT: Max-Flow/Minimum S-T Cut chua duoc trien khai.\n";
        return 0; // Attack FAIL la ket qua mo phong hop le, khong phai loi chuong trinh.
    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << '\n'; return 1;
    }
}
