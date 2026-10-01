#include "Node.h"
#include <stdexcept>
#include <utility>

Node::Node(int id, std::string name, NodeType type, int assets)
    : id(id), name(std::move(name)), type(type), assets(assets) {}
int Node::getID() const { return id; }
std::string Node::getName() const { return name; }
NodeType Node::getType() const { return type; }
int Node::getAssets() const { return assets; }
std::string nodeTypeToString(NodeType type) {
    switch (type) {
    case NodeType::ENTRY: return "ENTRY";
    case NodeType::ENDPOINT: return "ENDPOINT";
    case NodeType::IDENTITY: return "IDENTITY";
    case NodeType::CRITICAL_SYSTEM: return "CRITICAL_SYSTEM";
    case NodeType::TARGET: return "TARGET";
    }
    throw std::invalid_argument("NodeType khong hop le");
}
NodeType nodeTypeFromString(const std::string& text) {
    for (auto type : {NodeType::ENTRY, NodeType::ENDPOINT, NodeType::IDENTITY,
                      NodeType::CRITICAL_SYSTEM, NodeType::TARGET}) {
        if (nodeTypeToString(type) == text) return type;
    }
    throw std::invalid_argument("NodeType khong hop le: " + text);
}
