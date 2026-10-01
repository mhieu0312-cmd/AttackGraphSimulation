#ifndef ATTACKGRAPH_NODE_H
#define ATTACKGRAPH_NODE_H

#include <string>
using namespace std;

enum class NodeType {
    ENTRY,
    ENDPOINT,
    IDENTITY,
    CRITICAL_SYSTEM,
    TARGET
};

class Node {
private:
    int id;
    string name;
    NodeType type;
    int assets;

public:
    Node(int id, string name, NodeType type, int assets);

    int getID();
    string getName();
    NodeType getType();
    int getAssets();
};

#endif