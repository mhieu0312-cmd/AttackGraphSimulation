#pragma once

#include "Node.h"
#include "Edge.h"

#include <cstdint>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;


// =========================
// THONG KE GRAPH
// =========================

struct GraphStatistics {

    // So luong node va edge
    size_t nodes = 0;
    size_t edges = 0;

    // Edge dang mo va da bi block
    size_t activeEdges = 0;
    size_t blockedEdges = 0;

    // Bac vao va bac ra cua node
    map<int, size_t> inDegree;
    map<int, size_t> outDegree;

    // Thong ke theo loai
    map<string, size_t> nodeTypes;
    map<string, size_t> relations;
};


// =========================
// KET QUA KIEM TRA DUONG DI
// =========================

struct PathValidation {

    // Duong di co hop le khong
    bool valid = false;

    // Tong weight cua duong di
    int64_t cost = 0;

    // Thong bao ket qua
    string message;
};


// =========================
// GRAPH
// =========================

class Graph {

private:

    // Danh sach node
    vector<Node> Nodes;

    // Danh sach edge
    vector<Edge> Edges;

    // Luu cac edge di ra tu moi node
    // Gia tri la index cua Edge trong vector Edges
    unordered_map<int, vector<size_t>> Neighbors;

    // Tim nhanh vi tri Node theo ID
    unordered_map<int, size_t> nodeIndex;

    // Tim nhanh vi tri Edge theo ID
    unordered_map<int, size_t> edgeIndex;

    // Kiem tra node co ton tai
    void requireNode(int id) const;


public:

    // =========================
    // THEM NODE / EDGE
    // =========================

    void addNode(Node node);

    void addEdge(Edge edge);


    // =========================
    // TIM NODE / EDGE
    // =========================

    const Node* getNode(int id) const;

    const Edge* getEdge(int id) const;


    // Lay danh sach node
    const vector<Node>& getNodes() const {
        return Nodes;
    }

    // Lay danh sach edge
    const vector<Edge>& getEdges() const {
        return Edges;
    }


    // =========================
    // DANH SACH KE
    // =========================

    // Lay cac node co the di den
    vector<int> getNeighbor(int id) const;

    // Lay cac edge di ra va chua bi block
    vector<const Edge*> getOutGoingEdge(int id) const;


    // =========================
    // BLOCK EDGE
    // =========================

    // Kiem tra edge da bi block chua
    bool isBlocked(int edgeId) const;

    // Block mot edge
    void blockEdge(int edgeId);

    // Mo lai mot edge
    void unblockEdge(int edgeId);

    // Block nhieu edge
    void blockEdges(
        const vector<int>& edgeIds
    );


    // =========================
    // KIEM TRA DI CHUYEN
    // =========================

    // Kiem tra hacker co du token di qua edge
    bool canMove(
        int edgeId,
        int64_t remainingToken
    ) const;


    // =========================
    // DUYET GRAPH
    // =========================

    vector<int> bfs(int start) const;

    vector<int> dfs(int start) const;


    // =========================
    // THONG KE
    // =========================

    GraphStatistics statistics() const;


    // =========================
    // KIEM TRA DUONG DI
    // =========================

    PathValidation validatePath(
        int start,
        int target,
        const vector<int>& edgeIds
    ) const;


    // =========================
    // KIEM TRA DATASET
    // =========================

    vector<string> validationWarnings() const;
};