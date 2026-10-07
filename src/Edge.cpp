#include "Edge.h"

using namespace std;


// =========================
// CONSTRUCTOR
// =========================

Edge::Edge(
    int from,
    int to,
    int weight,
    string relation,
    bool blocked,
    int id,
    int capacity
)
    : from(from),
      to(to),
      weight(weight),
      relation(relation),
      blocked(blocked),
      id(id),
      capacity(capacity) {
}


// =========================
// GETTER
// =========================

int Edge::getID() const {
    return id;
}

int Edge::getFrom() const {
    return from;
}

int Edge::getTo() const {
    return to;
}

int Edge::getWeight() const {
    return weight;
}

string Edge::getRelation() const {
    return relation;
}

bool Edge::isBlocked() const {
    return blocked;
}

int Edge::getCapacity() const {
    return capacity;
}


// =========================
// SETTER
// =========================

void Edge::setBlocked(bool value) {
    blocked = value;
}