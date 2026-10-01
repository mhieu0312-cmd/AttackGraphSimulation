#include "Edge.h"
#include <utility>
Edge::Edge(int from, int to, int weight, std::string relation, bool blocked,
           int id, std::optional<int> capacity)
    : from(from), to(to), weight(weight), relation(std::move(relation)),
      blocked(blocked), id(id), capacity(capacity) {}
int Edge::getID() const { return id; }
int Edge::getFrom() const { return from; }
int Edge::getTo() const { return to; }
int Edge::getWeight() const { return weight; }
std::string Edge::getRelation() const { return relation; }
bool Edge::isBlocked() const { return blocked; }
std::optional<int> Edge::getCapacity() const { return capacity; }
void Edge::setBlocked(bool value) { blocked = value; }
