#pragma once

#include "Graph.h"

class Defender {

private:
    Graph& graph;

public:

    // Constructor
    explicit Defender(Graph& graph)
        : graph(graph) {
    }

    // Kiem tra source co den duoc target khong
    bool isReachable(int source, int target) const;

    // Tim mot edge don le co the ngat ket noi S -> T
    // Tra ve -1 neu khong tim thay
    int suggestCutEdge(int source, int target) const;

    // Block mot edge
    void blockEdge(int edgeId);
};