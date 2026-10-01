#pragma once

#include <string>

using namespace std;


// =========================
// LOAI NODE
// =========================

enum class NodeType {

    // Diem hacker bat dau
    ENTRY,

    // May tinh nguoi dung
    ENDPOINT,

    // Tai khoan hoac quyen truy cap
    IDENTITY,

    // Server / he thong quan trong
    CRITICAL_SYSTEM,

    // Muc tieu cuoi
    TARGET
};


// Chuyen NodeType thanh string
string nodeTypeToString(
    NodeType type
);

// Chuyen string thanh NodeType
NodeType nodeTypeFromString(
    const string& text
);


// =========================
// NODE
// =========================

class Node {

private:

    // ID cua node
    int id;

    // Ten node
    string name;

    // Loai cua node
    NodeType type;

    // Gia tri / muc do quan trong cua node
    int assets;


public:
    // Constructor
    Node(
        int id,
        string name,
        NodeType type,
        int assets
    );
    // Getter
    int getID() const;

    string getName() const;

    NodeType getType() const;

    int getAssets() const;
};