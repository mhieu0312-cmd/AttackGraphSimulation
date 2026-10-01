#include "LoadDataset.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <limits>
#include <set>
#include <stdexcept>

namespace {
using nlohmann::json;
int integer(const json& object, const std::string& key) {
    const auto& value = object.at(key);
    if (!value.is_number_integer()) throw std::invalid_argument(key + " phai la so nguyen");
    if (value.is_number_unsigned()) {
        auto number = value.get<std::uint64_t>();
        if (number > static_cast<std::uint64_t>(std::numeric_limits<int>::max()))
            throw std::invalid_argument(key + " vuot mien int");
        return static_cast<int>(number);
    }
    auto number = value.get<std::int64_t>();
    if (number < 0 || number > std::numeric_limits<int>::max())
        throw std::invalid_argument(key + " phai trong [0, INT_MAX]");
    return static_cast<int>(number);
}
void fields(const json& object, const std::set<std::string>& allowed) {
    if (!object.is_object()) throw std::invalid_argument("Can mot JSON object");
    for (auto it = object.begin(); it != object.end(); ++it)
        if (!allowed.count(it.key())) throw std::invalid_argument("Truong khong duoc ho tro: " + it.key());
}
}
Graph loadDataset(std::istream& input) {
    using nlohmann::json;
    Graph graph;
    try {
        // JSON chuan: khong comment, khong duplicate key.
        std::vector<std::set<std::string>> keys;
        auto checkKeys = [&keys](int, json::parse_event_t event, json& parsed) {
            if (event == json::parse_event_t::object_start) keys.emplace_back();
            if (event == json::parse_event_t::key && !keys.back().insert(parsed.get<std::string>()).second)
                throw std::invalid_argument("JSON co key trung: " + parsed.get<std::string>());
            if (event == json::parse_event_t::object_end) keys.pop_back();
            return true;
        };
        json data = json::parse(input, checkKeys);
        fields(data, {"metadata", "nodes", "edges"});
        if (!data.at("nodes").is_array() || !data.at("edges").is_array())
            throw std::invalid_argument("nodes/edges phai la array");
        std::size_t i = 0;
        for (const auto& n : data.at("nodes")) {
            try {
                fields(n, {"id", "name", "type", "assets"});
                graph.addNode(Node(integer(n, "id"), n.at("name").get<std::string>(),
                                   nodeTypeFromString(n.at("type").get<std::string>()), integer(n, "assets")));
            } catch (const std::exception& e) {
                throw std::invalid_argument("nodes[" + std::to_string(i) + "]: " + e.what());
            }
            ++i;
        }
        i = 0;
        for (const auto& e : data.at("edges")) {
            try {
                fields(e, {"id", "from", "to", "weight", "relation", "blocked", "capacity"});
                std::optional<int> capacity;
                if (e.contains("capacity") && !e.at("capacity").is_null()) capacity = integer(e, "capacity");
                bool blocked = e.contains("blocked") ? e.at("blocked").get<bool>() : false;
                graph.addEdge(Edge(integer(e, "from"), integer(e, "to"), integer(e, "weight"),
                                   e.at("relation").get<std::string>(), blocked, integer(e, "id"), capacity));
            } catch (const std::exception& ex) {
                throw std::invalid_argument("edges[" + std::to_string(i) + "]: " + ex.what());
            }
            ++i;
        }
    } catch (const std::exception& e) {
        throw std::invalid_argument(std::string("Dataset khong hop le: ") + e.what());
    }
    return graph;
}
Graph loadDataset(const std::string& filename) {
    std::ifstream input(filename);
    if (!input) throw std::runtime_error("Khong mo duoc dataset: " + filename);
    return loadDataset(input);
}
