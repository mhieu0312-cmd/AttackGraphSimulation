#pragma once

#include <optional>
#include <string>

using namespace std;

class Graph;


// =========================
// EDGE
// =========================

class Edge {

private:

    // Node bat dau va ket thuc
    int from;
    int to;

    // Chi phi hacker di qua edge
    int weight;

    // Loai quan he
    string relation;

    // Edge da bi Defender chan hay chua
    bool blocked;

    // ID cua edge
    int id;

    // Gia tri dung cho Min-Cut
    // Co the chua co capacity
    optional<int> capacity;

    // Cho phep Graph truy cap truc tiep
    friend class Graph;


public:

    // Constructor
    Edge(
        int from,
        int to,
        int weight,
        string relation,
        bool blocked,
        int id = -1,
        optional<int> capacity = nullopt
    );


    // Getter
    int getID() const;

    int getFrom() const;

    int getTo() const;

    int getWeight() const;

    string getRelation() const;

    bool isBlocked() const;

    optional<int> getCapacity() const;


    // Thay doi trang thai blocked
    void setBlocked(bool value);
};