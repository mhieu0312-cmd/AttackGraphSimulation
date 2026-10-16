#pragma once
#include <string>
using namespace std;

class Edge {

    int from;
    int to;

    // chi phi tan cong
    int weight;

    string relation;

    bool blocked;
    int id;

    // chi phi chan canh
    int capacity;

    friend class Graph;

  public:
    Edge(int from, int to, int weight, string relation, bool blocked, int id, int capacity);
    int getID() const;
    int getFrom() const;
    int getTo() const;
    int getWeight() const;
    string getRelation() const;
    bool isBlocked() const;
    int getCapacity() const;

    void setBlocked(bool value);
};
