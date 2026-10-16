#pragma once

#include "Graph.h"

#include <cstdint>
#include <iosfwd>
#include <optional>
#include <string>
#include <vector>

using namespace std;

struct PathResult {

    bool reachable = false;

    int64_t totalCost = 0;

    vector<int> nodes;

    vector<int> edgeIds;
};

enum class AttackStatus {

    SUCCESS,

    OVER_BUDGET,

    NO_PATH
};

struct AttackResult {
    // duong du kien, chua xet budget
    PathResult path;

    AttackStatus status = AttackStatus::NO_PATH;

    int64_t initialBudget = 0;

    // chi gom canh da di qua
    PathResult traveledPath;
    string stopReason;
    optional<int64_t> remainingToken;
};

PathResult dijkstra(const Graph& graph, int start, int target);

// sau patch, thu duong cu neu khong co duong moi
AttackResult hackerSimulation(const Graph& graph, int start, int target, int64_t budget,
                              const PathResult* previousRoute = nullptr);

bool canMove(const Graph& graph, int edgeId, int64_t remainingToken);

void printPath(const PathResult& path, ostream& out);
void printAttack(const AttackResult& attack, ostream& out);

void printPathDetailed(const Graph& graph, const PathResult& path, ostream& out);
void printAttackDetailed(const Graph& graph, const AttackResult& attack, ostream& out);
