#pragma once
#include "Graph.h"

class Defender {
    Graph& graph;
public:
    explicit Defender(Graph& graph) : graph(graph) {}
    bool isReachable(int source, int target) const;
    // Prototype goc: chi tim 1 canh ngat S-T. KHONG phai weighted Min-Cut.
    // -1: da mat ket noi, S==T, hoac khong co canh don le phu hop.
    int suggestCutEdge(int source, int target) const;
    void blockEdge(int edgeId);
};
