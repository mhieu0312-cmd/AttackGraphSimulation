#include "LoadDataset.h"

#include <nlohmann/json.hpp>
#include <fstream>
#include <stdexcept>
#include <limits>

using namespace std;
using nlohmann::json;


namespace {
// JSON get<int>() can truncate floats or overflow; validate before conversion.
int integerField(const json& object, const char* key) {
    const auto& value = object.at(key);
    if (value.is_number_unsigned()) {
        if (value.get<uint64_t>() > static_cast<uint64_t>(numeric_limits<int>::max())) {
            throw invalid_argument(string(key) + " vuot mien int");
        }
    } else if (value.is_number_integer()) {
        const auto number = value.get<int64_t>();
        if (number < numeric_limits<int>::min() || number > numeric_limits<int>::max()) {
            throw invalid_argument(string(key) + " vuot mien int");
        }
    } else {
        throw invalid_argument(string(key) + " phai la so nguyen");
    }
    return value.get<int>();
}
}

// =========================
// LOAD DATASET TU STREAM
// =========================

Graph loadDataset(istream& input) {

    Graph graph;

    try {

        json data;
        input >> data;

        // Kiem tra nodes va edges
        if (!data.contains("nodes") || !data["nodes"].is_array()) {
            throw invalid_argument("nodes khong hop le");
        }

        if (!data.contains("edges") || !data["edges"].is_array()) {
            throw invalid_argument("edges khong hop le");
        }


        // =========================
        // LOAD NODE
        // =========================

        for (const json& nodeData : data["nodes"]) {

            int id = integerField(nodeData, "id");

            string name =
                nodeData.at("name").get<string>();

            NodeType type =
                nodeTypeFromString(
                    nodeData.at("type").get<string>()
                );

            int assets =
                integerField(nodeData, "assets");

            Node node(
                id,
                name,
                type,
                assets
            );

            graph.addNode(node);
        }


        // =========================
        // LOAD EDGE
        // =========================

        for (const json& edgeData : data["edges"]) {

            int id =
                integerField(edgeData, "id");

            int from =
                integerField(edgeData, "from");

            int to =
                integerField(edgeData, "to");

            int weight =
                integerField(edgeData, "weight");

            string relation =
                edgeData.at("relation").get<string>();

            int capacity =
                integerField(edgeData, "capacity");

            bool blocked = false;

            if (edgeData.contains("blocked")) {
                blocked =
                    edgeData.at("blocked").get<bool>();
            }

            Edge edge(
                from,
                to,
                weight,
                relation,
                blocked,
                id,
                capacity
            );

            graph.addEdge(edge);
        }
    }

    catch (const exception& e) {

        throw invalid_argument(
            string("Dataset khong hop le: ")
            + e.what()
        );
    }

    return graph;
}


// =========================
// LOAD DATASET TU FILE
// =========================

Graph loadDataset(const string& filename) {

    ifstream input(filename);

    if (!input) {
        throw runtime_error(
            "Khong mo duoc dataset: "
            + filename
        );
    }

    return loadDataset(input);
}