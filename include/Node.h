#pragma once

#include <string>

using namespace std;




enum class NodeType {

    ENTRY,

    ENDPOINT,

    IDENTITY,

    CRITICAL_SYSTEM,

    TARGET
};


string nodeTypeToString(
    NodeType type
);

NodeType nodeTypeFromString(
    const string& text
);

// NODE
class Node {
    int id;
    string name;
    NodeType type;
    int assets;

public:
    Node(
        int id,
        string name,
        NodeType type,
        int assets
    );

    int getID() const;

    string getName() const;

    NodeType getType() const;

    int getAssets() const;
};