
#ifndef ATTACKGRAPH_EDGE_H
#define ATTACKGRAPH_EDGE_H

#include <string>
using namespace std;

class Edge {
private:
    int from;
    int to;
    int weight;
    string relation;
    bool blocked;

public:
    Edge(int from, int to, int weight,
         string relation, bool blocked);

    int getFrom();
    int getTo();
    int getWeight();
    string getRelation();

    bool isBlocked();
    void setBlocked(bool blocked);
};

#endif