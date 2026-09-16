#include "../include/Node.h"

Node::Node(int id, string name, NodeType type, int assets) {
    this->id = id;
    this->name = name;
    this->type = type;
    this->assets = assets;
}

int Node::getID() {
    return id;
}

string Node::getName() {
    return name;
}

NodeType Node::getType() {
    return type;
}

int Node::getAssets() {
    return assets;
}
