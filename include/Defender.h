#pragma once

#include "Graph.h"

class Defender {
  private:
    Graph& graph;

  public:
    explicit Defender(Graph& graph) : graph(graph) {}

    bool isReachable(int source, int target) const;

    int suggestCutEdge(int source, int target) const;

    void blockEdge(int edgeId);
};
