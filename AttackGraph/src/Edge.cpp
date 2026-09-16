//
// Created by mhieu on 9/16/2026.
//
*989
#include "Edge.h"
Edge::Edge(int from, int to, int weight,
           string relation, bool blocked) {

    this->from = from;
    this->to = to;
    this->weight = weight;
    this->relation = relation;
    this->blocked = blocked;
}

int Edge::getFrom() {
    return from;
}

int Edge::getTo() {
    return to;
}

int Edge::getWeight() {
    return weight;
}

string Edge::getRelation() {
    return relation;
}

bool Edge::isBlocked() {
    return blocked;
}

void Edge::setBlocked(bool blocked) {
    this->blocked = blocked;
}