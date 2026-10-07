#pragma once
#include <optional>
#include <string>
using namespace std;

class Edge {
    // Node bat dau, ket thuc
    int from;
    int to;

    // trong so cho hacker
    int weight;

    // Loai quan he
    string relation;

    // Edge da bi Defender chan hay chua
    bool blocked;
    int id;

    // Gia tri dung cho Min-Cut
    int capacity;

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
        int id,
        int capacity
    );

    int getID() const;

    int getFrom() const;

    int getTo() const;

    int getWeight() const;

    string getRelation() const;

    bool isBlocked() const;

    int getCapacity() const;


    // Thay doi trang thai blocked
    void setBlocked(bool value);
};