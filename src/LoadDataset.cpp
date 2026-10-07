#include "LoadDataset.h"

#include <nlohmann/json.hpp>
#include <fstream>
#include <stdexcept>

using namespace std;
using nlohmann::json;


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

            int id = nodeData.at("id").get<int>();

            string name =
                nodeData.at("name").get<string>();

            NodeType type =
                nodeTypeFromString(
                    nodeData.at("type").get<string>()
                );

            int assets =
                nodeData.at("assets").get<int>();

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
                edgeData.at("id").get<int>();

            int from =
                edgeData.at("from").get<int>();

            int to =
                edgeData.at("to").get<int>();

            int weight =
                edgeData.at("weight").get<int>();

            string relation =
                edgeData.at("relation").get<string>();

            int capacity =
                edgeData.at("capacity").get<int>();

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