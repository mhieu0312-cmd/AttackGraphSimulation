#include "RuntimeInput.h"
#include <algorithm>
#include <iostream>
#include <limits>
#include <stdexcept>

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
