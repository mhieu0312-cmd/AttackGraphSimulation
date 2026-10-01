#pragma once
#include "Node.h"
#include "Edge.h"
#include <cstdint>
#include <map>
#include <unordered_map>
#include <vector>

struct GraphStatistics {
    std::size_t nodes = 0, edges = 0, activeEdges = 0, blockedEdges = 0;
    std::map<int, std::size_t> inDegree, outDegree; // Chi tinh canh dang mo.
    std::map<std::string, std::size_t> nodeTypes, relations;
};
struct PathValidation {
    bool valid = false;
    std::int64_t cost = 0;
    std::string message;
};

class Graph {
    std::vector<Node> Nodes;
    std::vector<Edge> Edges;
    // Luu INDEX canh, loc blocked khi truy van; khong cache danh sach active.
    std::unordered_map<int, std::vector<std::size_t>> Neighbors;
    std::unordered_map<int, std::size_t> nodeIndex, edgeIndex;
    void requireNode(int id) const;
public:
    void addNode(Node node);
    void addEdge(Edge edge);
    const Node* getNode(int id) const;
    const Edge* getEdge(int id) const;
    const std::vector<Node>& getNodes() const { return Nodes; }
    const std::vector<Edge>& getEdges() const { return Edges; }
    std::vector<int> getNeighbor(int id) const;
    std::vector<const Edge*> getOutGoingEdge(int id) const;
    bool isBlocked(int edgeId) const;
    void blockEdge(int edgeId);
    void unblockEdge(int edgeId);
    void blockEdges(const std::vector<int>& edgeIds);
    bool canMove(int edgeId, std::int64_t remainingToken) const;
    std::vector<int> bfs(int start) const;
    std::vector<int> dfs(int start) const;
    GraphStatistics statistics() const;
    // Tra canh theo ID de phan biet cac canh song song.
    PathValidation validatePath(int start, int target,
                                const std::vector<int>& edgeIds) const;
    std::vector<std::string> validationWarnings() const;
};
