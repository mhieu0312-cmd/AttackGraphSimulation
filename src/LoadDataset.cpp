#include "LoadDataset.h"

#include <nlohmann/json.hpp>
#include <fstream>
#include <limits>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;
using nlohmann::json;


// =========================
// DOC SO NGUYEN
// =========================

namespace {

int getInteger(
    const json& object,
    const string& key
) {

    const json& value = object.at(key);

    // Gia tri phai la so nguyen
    if (!value.is_number_integer()) {
        throw invalid_argument(
            key + " phai la so nguyen"
        );
    }

    // Truong hop so khong dau
    if (value.is_number_unsigned()) {

        uint64_t number =
            value.get<uint64_t>();

        if (number >
            static_cast<uint64_t>(
                numeric_limits<int>::max()
            )) {

            throw invalid_argument(
                key + " vuot mien int"
            );
        }

        return static_cast<int>(number);
    }


    // Truong hop so co dau
    int64_t number =
        value.get<int64_t>();

    if (number < 0 ||
        number > numeric_limits<int>::max()) {

        throw invalid_argument(
            key + " phai trong [0, INT_MAX]"
        );
    }

    return static_cast<int>(number);
}


// =========================
// KIEM TRA CAC FIELD
// =========================

void checkFields(
    const json& object,
    const set<string>& allowed
) {

    // Du lieu phai la JSON object
    if (!object.is_object()) {
        throw invalid_argument(
            "Can mot JSON object"
        );
    }

    // Kiem tra field la
    for (auto it = object.begin();
         it != object.end();
         ++it) {

        if (!allowed.count(it.key())) {

            throw invalid_argument(
                "Truong khong duoc ho tro: "
                + it.key()
            );
        }
    }
}

}


// =========================
// LOAD DATASET TU STREAM
// =========================

Graph loadDataset(istream& input) {

    Graph graph;

    try {

        // Kiem tra duplicate key trong JSON
        vector<set<string>> keys;

        auto checkKeys =
            [&keys](
                int,
                json::parse_event_t event,
                json& parsed
            ) {

                if (event ==
                    json::parse_event_t::object_start) {

                    keys.emplace_back();
                }


                if (event ==
                    json::parse_event_t::key) {

                    string key =
                        parsed.get<string>();

                    if (!keys.back().insert(key).second) {

                        throw invalid_argument(
                            "JSON co key trung: "
                            + key
                        );
                    }
                }


                if (event ==
                    json::parse_event_t::object_end) {

                    keys.pop_back();
                }


                return true;
            };


        // Doc JSON
        json data =
            json::parse(input, checkKeys);


        // Cac field cho phep o root
        checkFields(
            data,
            {
                "metadata",
                "nodes",
                "edges"
            }
        );


        // nodes va edges phai la array
        if (!data.at("nodes").is_array() ||
            !data.at("edges").is_array()) {

            throw invalid_argument(
                "nodes/edges phai la array"
            );
        }


        // =========================
        // LOAD NODE
        // =========================

        size_t i = 0;

        for (const json& nodeData :
             data.at("nodes")) {

            try {

                checkFields(
                    nodeData,
                    {
                        "id",
                        "name",
                        "type",
                        "assets"
                    }
                );


                Node node(
                    getInteger(nodeData, "id"),

                    nodeData.at("name")
                        .get<string>(),

                    nodeTypeFromString(
                        nodeData.at("type")
                            .get<string>()
                    ),

                    getInteger(
                        nodeData,
                        "assets"
                    )
                );


                graph.addNode(node);
            }

            catch (const exception& e) {

                throw invalid_argument(
                    "nodes["
                    + to_string(i)
                    + "]: "
                    + e.what()
                );
            }


            ++i;
        }


        // =========================
        // LOAD EDGE
        // =========================

        i = 0;

        for (const json& edgeData :
             data.at("edges")) {

            try {

                checkFields(
                    edgeData,
                    {
                        "id",
                        "from",
                        "to",
                        "weight",
                        "relation",
                        "blocked",
                        "capacity"
                    }
                );


                // Capacity co the khong co
                optional<int> capacity;

                if (edgeData.contains("capacity") &&
                    !edgeData.at("capacity").is_null()) {

                    capacity =
                        getInteger(
                            edgeData,
                            "capacity"
                        );
                }


                // blocked mac dinh la false
                bool blocked = false;

                if (edgeData.contains("blocked")) {

                    blocked =
                        edgeData.at("blocked")
                            .get<bool>();
                }


                Edge edge(
                    getInteger(edgeData, "from"),
                    getInteger(edgeData, "to"),
                    getInteger(edgeData, "weight"),

                    edgeData.at("relation")
                        .get<string>(),

                    blocked,

                    getInteger(edgeData, "id"),

                    capacity
                );


                graph.addEdge(edge);
            }

            catch (const exception& e) {

                throw invalid_argument(
                    "edges["
                    + to_string(i)
                    + "]: "
                    + e.what()
                );
            }


            ++i;
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

Graph loadDataset(
    const string& filename
) {

    ifstream input(filename);

    // Kiem tra file co mo duoc khong
    if (!input) {

        throw runtime_error(
            "Khong mo duoc dataset: "
            + filename
        );
    }

    return loadDataset(input);
}