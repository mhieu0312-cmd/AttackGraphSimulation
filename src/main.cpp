#include "LoadDataset.h"
#include "Simulation.h"

#include <iostream>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

namespace {

// Chuyen tham so thanh so nguyen khong am
int64_t number(const char* text) {

    string s(text);

    if (s.empty() ||
        s.find_first_not_of("0123456789") != string::npos) {

        throw invalid_argument(
            "Tham so phai la so nguyen khong am"
        );
    }

    size_t end = 0;
    int64_t value = stoll(s, &end);

    if (end != s.size()) {
        throw invalid_argument(
            "Tham so khong hop le"
        );
    }

    return value;
}


// Chuyen tham so thanh ID
int idNumber(const char* text) {

    int64_t value = number(text);

    if (value > numeric_limits<int>::max()) {
        throw invalid_argument(
            "ID vuot mien int"
        );
    }

    return static_cast<int>(value);
}


// Huong dan cach chay
void usage() {

    cout
        << "Usage:\n"
        << "  AttackGraph\n"
        << "  AttackGraph --demo\n"
        << "  AttackGraph <dataset> <source> <target> <budget>\n"
        << "  AttackGraph <dataset> <source> <target> <budget> "
           "--patch <edgeId...>\n";
}

}

int main(int argc, char* argv[]) {

    try {

        // =========================
        // CAU HINH MAC DINH
        // =========================

        string filename = string(PROJECT_ROOT) + "/data/graph.json";

        // Sửa source thành 37, target thành 1 (thay vì 0 và 37)
        int source = 37;
        int target = 1;
        int64_t budget = 15;

        optional<vector<int>> manual;
        // =========================
        // HELP
        // =========================
        if (argc == 2 && string(argv[1]) == "--help") {
            usage();
            return 0;
        }
        // Thêm xử lý cho cờ --demo
        if (argc == 2 && string(argv[1]) == "--demo") {
            // Giữ nguyên giá trị mặc định (37 -> 1, budget 15) để chạy demo
        }
        // =========================
        // THAM SO TU NGUOI DUNG
        // =========================
        else if (argc > 1) {

            if (argc < 5) {
                usage();
                return 1;
            }

            filename = argv[1];

            source = idNumber(argv[2]);
            target = idNumber(argv[3]);
            budget = number(argv[4]);

            // Patch thu cong
            if (argc > 5) {

                if (string(argv[5]) != "--patch" || argc == 6) {
                    throw invalid_argument(
                        "Can --patch va it nhat mot edge ID"
                    );
                }

                manual = vector<int>{};

                for (int i = 6; i < argc; ++i) {
                    manual->push_back(
                        idNumber(argv[i])
                    );
                }
            }
        }


        // =========================
        // LOAD DATASET
        // =========================

        Graph graph = loadDataset(filename);

        auto stats = graph.statistics();

        cout << "\n=== DATASET ===\n";
        cout << "File: " << filename << '\n';
        cout << "Nodes: " << stats.nodes << '\n';
        cout << "Edges: " << stats.edges << '\n';
        cout << "Source: " << source << '\n';
        cout << "Target: " << target << '\n';
        cout << "Budget: " << budget << '\n';


        // Kiem tra dataset
        for (const auto& warning :
             graph.validationWarnings()) {

            cout
                << "Warning: "
                << warning
                << '\n';
        }


        // =========================
        // CHAY MO PHONG
        // =========================

        auto result = runSimulation(
            graph,
            source,
            target,
            budget,
            manual
        );


        // =========================
        // TRUOC KHI PATCH
        // =========================

        cout << "\n=== BEFORE PATCH ===\n";

        printAttack(
            result.before,
            cout
        );


        // =========================
        // PHONG THU
        // =========================

        cout << "\n=== DEFENSE ===\n";

        cout
            << result.defenseMessage
            << '\n';

        cout << "Patched edges: ";

        if (result.patchedEdgeIds.empty()) {

            cout << "none";
        }
        else {

            for (int id :
                 result.patchedEdgeIds) {

                cout << id << ' ';
            }
        }

        cout << '\n';


        // =========================
        // SAU KHI PATCH
        // =========================

        cout << "\n=== AFTER PATCH ===\n";

        printAttack(
            result.after,
            cout
        );


        // =========================
        // KET QUA
        // =========================

        cout << "\n=== RESULT ===\n";

        cout
            << "Reachable: "
            << result.reachableBefore
            << " -> "
            << result.reachableAfter
            << '\n';

        cout
            << "Blocked edges: "
            << stats.blockedEdges
            << " -> "
            << graph.statistics().blockedEdges
            << '\n';

        return 0;
    }


    // =========================
    // BAT LOI
    // =========================

    catch (const exception& e) {

        cerr
            << "ERROR: "
            << e.what()
            << '\n';

        return 1;
    }
}
