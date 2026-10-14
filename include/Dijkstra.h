#pragma once

#include "Graph.h"

#include <cstdint>
#include <iosfwd>
#include <optional>
#include <string>
#include <vector>

using namespace std;

// KET QUA DUONG DI

struct PathResult {
    // Co duong den target hay khong
    bool reachable = false;

    // Tong weight cua duong di
    int64_t totalCost = 0;

    // Danh sach node tren duong di
    vector<int> nodes;

    // Danh sach edge tren duong di
    vector<int> edgeIds;
};

// TRANG THAI TAN CONG

enum class AttackStatus {
    // Hacker den duoc target
    SUCCESS,

    // Co duong nhung khong du token
    OVER_BUDGET,

    // Khong co duong den target
    NO_PATH
};

// KET QUA TAN CONG

struct AttackResult {
    // Duong Dijkstra du kien, chua xet budget
    PathResult path;

    // Trang thai tan cong
    AttackStatus status = AttackStatus::NO_PATH;

    // Token ban dau
    int64_t initialBudget = 0;

    // Hanh trinh thuc te, chi gom canh da di qua duoc.
    PathResult traveledPath;
    string stopReason;
    optional<int64_t> remainingToken;
};

// DIJKSTRA

// Tim duong co tong weight nho nhat
PathResult dijkstra(const Graph& graph, int start, int target);

// HACKER SIMULATION

// Dijkstra tim lo trinh; hacker chi di khi canh con mo va du token.
// Neu NO_PATH sau Patch, thu lai lo trinh cu va dung tai canh khong di duoc.
AttackResult hackerSimulation(const Graph& graph, int start, int target, int64_t budget,
                              const PathResult* previousRoute = nullptr);

// DIEU KIEN DI CHUYEN (CAN MOVE)

// Canh phai ton tai, chua block va du token de di qua.
bool canMove(const Graph& graph, int edgeId, int64_t remainingToken);

// IN KET QUA

void printPath(const PathResult& path, ostream& out);
void printAttack(const AttackResult& attack, ostream& out);

// In ket qua truc quan gom ten Node va Relation giup bao cao va giai thich
void printPathDetailed(const Graph& graph, const PathResult& path, ostream& out);
void printAttackDetailed(const Graph& graph, const AttackResult& attack, ostream& out);
