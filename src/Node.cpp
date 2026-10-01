#include "Node.h"

#include <stdexcept>
#include <utility>

using namespace std;


// =========================
// CONSTRUCTOR
// =========================

Node::Node(
    int id,
    string name,
    NodeType type,
    int assets
)
    : id(id),
      name(move(name)),
      type(type),
      assets(assets) {
}


// =========================
// GETTER
// =========================

int Node::getID() const {
    return id;
}

string Node::getName() const {
    return name;
}

NodeType Node::getType() const {
    return type;
}

int Node::getAssets() const {
    return assets;
}


// =========================
// NODE TYPE -> STRING
// =========================

string nodeTypeToString(NodeType type) {

    switch (type) {

        case NodeType::ENTRY:
            return "ENTRY";

        case NodeType::ENDPOINT:
            return "ENDPOINT";

        case NodeType::IDENTITY:
            return "IDENTITY";

        case NodeType::CRITICAL_SYSTEM:
            return "CRITICAL_SYSTEM";

        case NodeType::TARGET:
            return "TARGET";
    }

    throw invalid_argument(
        "NodeType khong hop le"
    );
}


// =========================
// STRING -> NODE TYPE
// =========================

NodeType nodeTypeFromString(
    const string& text
) {

    // Kiem tra tung NodeType
    for (NodeType type : {
        NodeType::ENTRY,
        NodeType::ENDPOINT,
        NodeType::IDENTITY,
        NodeType::CRITICAL_SYSTEM,
        NodeType::TARGET
    }) {

        if (nodeTypeToString(type) == text) {
            return type;
        }
    }

    throw invalid_argument(
        "NodeType khong hop le: " + text
    );
}