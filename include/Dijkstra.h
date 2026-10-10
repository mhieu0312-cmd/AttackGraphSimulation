#pragma once

#include "Graph.h"

#include <cstdint>
#include <iosfwd>
#include <optional>
#include <vector>

using namespace std;


// =========================
// KET QUA DUONG DI
// =========================

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


// =========================
// TRANG THAI TAN CONG
// =========================

enum class AttackStatus {

    // Hacker den duoc target
    SUCCESS,

    // Co duong nhung khong du token
    OVER_BUDGET,

    // Khong co duong den target
    NO_PATH
};


// =========================
// KET QUA TAN CONG
// =========================

struct AttackResult {

    // Duong di cua hacker
    PathResult path;

    // Trang thai tan cong
    AttackStatus status = AttackStatus::NO_PATH;

    // Token ban dau
    int64_t initialBudget = 0;

    // Token con lai neu tan cong thanh cong
    optional<int64_t> remainingToken;
};


// =========================
// DIJKSTRA
// =========================

// Tim duong co tong weight nho nhat
PathResult dijkstra(
    const Graph& graph,
    int start,
    int target
);


// =========================
// HACKER SIMULATION
// =========================

// Mo phong hacker voi budget cho truoc
AttackResult hackerSimulation(
    const Graph& graph,
    int start,
    int target,
    int64_t budget
);


// =========================
// DIEU KIEN DI CHUYEN (CAN MOVE)
// =========================

// Kiem tra hacker co the di qua mot canh cu the voi so token hien co hay khong:
// 1. Canh phai ton tai trong do thi
// 2. Canh chua bi Defender chan (!isBlocked)
// 3. So token con lai phai khong am va >= weight cua canh
bool canMove(
    const Graph& graph,
    int edgeId,
    int64_t remainingToken
);


// =========================
// IN KET QUA
// =========================

void printPath(
    const PathResult& path,
    ostream& out
);

void printAttack(
    const AttackResult& attack,
    ostream& out
);

// In ket qua truc quan gom ten Node va Relation giup bao cao va giai thich
void printPathDetailed(
    const Graph& graph,
    const PathResult& path,
    ostream& out
);

void printAttackDetailed(
    const Graph& graph,
    const AttackResult& attack,
    ostream& out
);