#pragma once
#include <optional>
#include <string>

class Graph;
class Edge {
    int from, to, weight;
    std::string relation;
    bool blocked;
    int id;
    // nullopt: chua co chinh sach chi phi phong thu. Khong lay weight thay the.
    std::optional<int> capacity;
    friend class Graph;
public:
    Edge(int from, int to, int weight, std::string relation, bool blocked,
         int id = -1, std::optional<int> capacity = std::nullopt);
    int getID() const;
    int getFrom() const;
    int getTo() const;
    int getWeight() const;
    std::string getRelation() const;
    bool isBlocked() const;
    std::optional<int> getCapacity() const;
    void setBlocked(bool value);
};
