#pragma once
#include "Graph.h"
#include <cstdint>
#include <filesystem>
#include <iosfwd>

struct SimulationParameters {
    int source;
    int target;
    std::int64_t budget;
};

// Explicit path never falls back. Source-tree data takes priority in development.
std::filesystem::path resolveDatasetPath(const std::filesystem::path& explicitPath,
    const std::filesystem::path& executable, const std::filesystem::path& projectRoot);
std::int64_t parseNonnegative(const std::string& text);
int parseNodeId(const std::string& text);
void validateParameters(const Graph& graph, const SimulationParameters& parameters);
SimulationParameters selectParameters(const Graph& graph, std::istream& input, std::ostream& output);
// Noninteractive demo selects first ENTRY/TARGET in file order, budget 0.
SimulationParameters demoParameters(const Graph& graph);
