#pragma once
#include <string>

enum class NodeType { ENTRY, ENDPOINT, IDENTITY, CRITICAL_SYSTEM, TARGET };
std::string nodeTypeToString(NodeType type);
NodeType nodeTypeFromString(const std::string& text);

class Node {
    int id;
    std::string name;
    NodeType type;
    int assets;
public:
    Node(int id, std::string name, NodeType type, int assets);
    int getID() const;
    std::string getName() const;
    NodeType getType() const;
    int getAssets() const;
};
