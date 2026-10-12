#include "LoadDataset.h"

#include <nlohmann/json.hpp>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <vector>

using namespace std;
using nlohmann::json;

namespace {

void validateFields(const json& object, const vector<string>& allowedFields) {
    if (!object.is_object()) {
        throw invalid_argument("Ban ghi phai la object");
    }
    for (auto field = object.begin(); field != object.end(); ++field) {
        bool allowed = false;
        for (const string& name : allowedFields) {
            if (field.key() == name) {
                allowed = true;
                break;
            }
        }
        if (!allowed) {
            throw invalid_argument("Truong khong duoc ho tro: " + field.key());
        }
    }
}

int integerField(const json& object, const char* key) {
    const json& value = object.at(key);
    if (value.is_number_unsigned()) {
        if (value.get<uint64_t>() > static_cast<uint64_t>(numeric_limits<int>::max())) {
            throw invalid_argument(string(key) + " vuot mien int");
        }
    } else if (value.is_number_integer()) {
        int64_t number = value.get<int64_t>();
        if (number < 0 || number > numeric_limits<int>::max()) {
            throw invalid_argument(string(key) + " phai nam trong 0..INT_MAX");
        }
    } else {
        throw invalid_argument(string(key) + " phai la so nguyen");
    }
    return value.get<int>();
}

json readJson(istream& input) {
    vector<vector<string>> objectKeys;

    // Callback kiem tra key trung truoc khi thu vien ghi de gia tri cu.
    auto checkKeys = [&objectKeys](int, json::parse_event_t event, json& value) {
        if (event == json::parse_event_t::object_start) {
            objectKeys.push_back(vector<string>{});
        } else if (event == json::parse_event_t::object_end) {
            objectKeys.pop_back();
        } else if (event == json::parse_event_t::key) {
            string key = value.get<string>();
            for (const string& previousKey : objectKeys.back()) {
                if (previousKey == key) {
                    throw invalid_argument("JSON co key trung: " + key);
                }
            }
            objectKeys.back().push_back(key);
        }
        return true;
    };
    return json::parse(input, checkKeys);
}

}

Graph loadDataset(istream& input) {
    Graph graph;
    string location = "root";

    try {
        json data = readJson(input);
        validateFields(data, {"nodes", "edges", "metadata"});
        if (!data.contains("nodes") || !data.at("nodes").is_array()) {
            throw invalid_argument("nodes phai la array");
        }
        if (!data.contains("edges") || !data.at("edges").is_array()) {
            throw invalid_argument("edges phai la array");
        }
        if (data.contains("metadata") && !data.at("metadata").is_object()) {
            throw invalid_argument("metadata phai la object");
        }

        for (size_t index = 0; index < data.at("nodes").size(); ++index) {
            location = "nodes[" + to_string(index) + "]";
            const json& nodeData = data.at("nodes").at(index);
            validateFields(nodeData, {"id", "name", "type", "assets"});

            int id = integerField(nodeData, "id");
            string name = nodeData.at("name").get<string>();
            NodeType type = nodeTypeFromString(nodeData.at("type").get<string>());
            int assets = integerField(nodeData, "assets");
            graph.addNode(Node(id, name, type, assets));
        }

        for (size_t index = 0; index < data.at("edges").size(); ++index) {
            location = "edges[" + to_string(index) + "]";
            const json& edgeData = data.at("edges").at(index);
            validateFields(edgeData, {"id", "from", "to", "weight", "capacity", "relation", "blocked"});

            int id = integerField(edgeData, "id");
            int from = integerField(edgeData, "from");
            int to = integerField(edgeData, "to");
            int weight = integerField(edgeData, "weight");
            int capacity = integerField(edgeData, "capacity");
            string relation = edgeData.at("relation").get<string>();
            bool blocked = false;
            if (edgeData.contains("blocked")) {
                blocked = edgeData.at("blocked").get<bool>();
            }
            graph.addEdge(Edge(from, to, weight, relation, blocked, id, capacity));
        }
    } catch (const exception& error) {
        throw invalid_argument("Dataset khong hop le tai " + location + ": " + error.what());
    }
    return graph;
}

Graph loadDataset(const string& filename) {
    ifstream input(filename);
    if (!input) {
        throw runtime_error("Khong mo duoc dataset: " + filename);
    }
    return loadDataset(input);
}
