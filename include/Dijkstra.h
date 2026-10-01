#pragma once
#include "Graph.h"
#include <iosfwd>
#include <optional>

struct PathResult {
    bool reachable = false;
    std::int64_t totalCost = 0; // Chi co nghia khi reachable=true.
    std::vector<int> nodes;
    std::vector<int> edgeIds;
};
enum class AttackStatus { SUCCESS, OVER_BUDGET, NO_PATH };
struct AttackResult {
    PathResult path;
    AttackStatus status = AttackStatus::NO_PATH;
    std::int64_t initialBudget = 0;
    // Chi tru token khi co duong va du tien de di tron duong.
    std::optional<std::int64_t> remainingToken;
};

PathResult dijkstra(const Graph& graph, int start, int target);
AttackResult hackerSimulation(const Graph& graph, int start, int target, std::int64_t budget);
void printPath(const PathResult& path, std::ostream& out);
void printAttack(const AttackResult& attack, std::ostream& out);
