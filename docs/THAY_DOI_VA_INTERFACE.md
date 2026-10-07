# Lỗi đã sửa và thay đổi interface

## Lỗi quan trọng của bản gốc

Các đường dẫn trong cột đầu là đường dẫn **bên trong ZIP gốc**, cũng có bản sao tại `reference/original/`.

| File/hàm | Nguyên nhân | Ảnh hưởng | Cách sửa trong bản bàn giao |
|---|---|---|---|
| `AttackGraph/src/Edge.cpp`, dòng 4 | Có ký tự `*989` ngoài hàm | Build Graph thất bại, kéo theo nhiều lỗi compiler thứ cấp | Xóa ký tự thừa; triển khai getter const |
| `Graph::addNode/addEdge` | Chỉ push, không validate | ID trùng, cạnh trỏ node thiếu, weight âm lọt vào thuật toán | Validate trước khi thêm; index theo ID; không giới hạn ID liên tiếp |
| `Graph::getNeighbor` | Đọc cache node đích không lọc blocked; `Neighbors[id]` thêm key khi chỉ đọc | Patch không ảnh hưởng truy vấn; ID sai trông giống node cô lập | Cache index cạnh; truy vấn lọc trạng thái hiện tại; thiếu ID thì báo lỗi |
| `Graph::getOutGoingEdge` | Không kiểm tra blocked | Attack/Connectivity vẫn có thể đi qua cạnh vá | Trả các cạnh active; đọc const |
| `Edge` | Không có ID/capacity | P3 có ID nhưng P1 không có điểm nối; không thể định nghĩa weighted cut | Thêm ID ổn định và capacity tùy chọn, giữ weight riêng |
| `p2_hacker_dijkstra.cpp`: globals/MAX_NODE/INF | Mảng 40, ID là index, INF=1e9, tổng cost int | Không chạy an toàn với ID thưa hoặc >40 node; cost ≥1e9 có thể bị hiểu là vô cực, tổng có thể tràn | Dùng Graph chung, ánh xạ ID→index, vector và int64; giữ thuật toán O(V²) |
| `p2_hacker_dijkstra.cpp`: cost[u][v] | Mỗi cặp u,v chỉ giữ một cost và một blocked | Không phân biệt cạnh song song, patch theo cặp có thể chặn sai cạnh | Relax cạnh theo edge ID; parentEdge để tái tạo đúng đường |
| `hackerSimulation` | Chỉ in kết quả, trạng thái global tách rời P1 | Khó kiểm thử/so sánh Before–After; sửa Graph P1 không cập nhật ma trận P2 | Trả AttackResult; hàm nhận const Graph& mỗi lần |
| `Src/main.cpp` và `Src/Defender.h/.cpp` | Định nghĩa Edge/Defender trùng tên nhưng class body khác giữa translation unit | Vi phạm One Definition Rule; linker có thể chọn lẫn implementation dù demo vẫn chạy | Một Defender.h/.cpp duy nhất, một model Edge chung; demo mới chỉ gọi API |
| `Defender::isReachable` | visited size=numNodes nhưng index trực tiếp source/target/u/v | ID không hợp lệ có thể truy cập ngoài mảng | Kiểm tra ID và duyệt qua Graph, visited theo ID |
| `Defender::suggestCutEdge` | Không kiểm tra S–T đã mất kết nối trước khi thử | Có thể đề xuất cắt một cạnh không liên quan | Nếu vốn không reachable hoặc S==T thì không đề xuất |
| `Defender::suggestCutEdge` và output | Tên “Min-Cut”, “optimal”, “chi phí thấp nhất” nhưng chỉ thử từng cạnh | Kết luận sai khi cần cắt nhiều cạnh hoặc capacity khác nhau | Ghi rõ single-edge cut prototype; không triển khai thay Max-Flow/Min-Cut |
| `Defender::blockEdge` | Chặn graph nội bộ khác Graph P1 | Re-test trên Graph P1 không thấy patch | Defender giữ Graph& và gọi Graph::blockEdge |
| `getNode/getOutGoingEdge` | Trả con trỏ vào vector | addNode/addEdge có thể làm mất hiệu lực con trỏ đã giữ | Query chỉ đọc; thuật toán lưu ID/index, không giữ pointer qua thêm dữ liệu; quy tắc lifetime bên dưới |
| Cấu trúc build | Hai CMake project riêng, main Graph rỗng, P2 một file có main | Chưa có target tích hợp; gom bằng wildcard dễ kéo nhiều main | Root CMake liệt kê source rõ, core library + demo + test riêng |
| Cache build được nén kèm | CMakeCache/CMakeFiles của máy Windows cũ, có đường dẫn riêng | Không dùng được như build mới trên máy khác | Không đóng gói cache/binary vào bản mới; source tham chiếu cũng loại cache |

Source P2 **đã đúng** ở hai điểm: dùng `cost != INF` nên cạnh weight 0 được nhận diện; dùng `minCost <= budget` nên bằng budget được phép đi. Bản mới giữ hai hành vi này và thêm test. Không quy lỗi không tồn tại cho code gốc.

Trong các target gốc không có tình trạng hai hàm main cùng một target; vấn đề là chưa có target chung và có hai định nghĩa Defender. CMake mới không đưa `reference/original` vào build.

## Danh sách file sửa/thêm/di chuyển

| File bàn giao | Thay đổi và lý do |
|---|---|
| `include/Node.h`, `src/Node.cpp` | Giữ trường/constructor gốc, getter const, chuyển đổi enum↔chuỗi phục vụ JSON |
| `include/Edge.h`, `src/Edge.cpp` | Sửa compile, thêm ID/capacity, getter const; giữ constructor 5 tham số hợp lệ |
| `include/Graph.h`, `src/Graph.cpp` | Validation, index ID/adjacency active, BFS/DFS, thống kê, path/canMove, batch Patch |
| `include/Dijkstra.h`, `src/Dijkstra.cpp` | Lớp rỗng được thay bằng API thực; chuyển logic từ P2, tách main và globals; trả kết quả |
| `include/Defender.h`, `src/Defender.cpp` | Chuyển prototype từ Src, dùng chung Graph, giữ BFS và thử cắt đơn lẻ, sửa trường hợp đã mất kết nối |
| `include/LoadDataset.h`, `src/LoadDataset.cpp` | Mới; load JSON theo schema rõ ràng, lỗi có ngữ cảnh |
| `include/Simulation.h`, `src/Simulation.cpp` | Mới; lưu hai kết quả, nối đề xuất/patch thật, reset budget |
| `src/main.cpp` | Thay main rỗng bằng CLI, thống kê và Before/After |
| `CMakeLists.txt` | Root build chung; `attackgraph_core`, `AttackGraph`, `attackgraph_tests`; đăng ký CTest |
| `.gitignore` | Loại cache IDE/build/object khỏi lần commit sau |
| `tests/test_main.cpp` | Mới; 47 test và kiểm tra đối chiếu Dijkstra bằng Bellman-Ford độc lập trong test |
| `tests/fixtures/p2_demo.json` | Mới; chuyển demo P2 có sẵn thành JSON fixture, không thay dataset thật |
| `data/README.md` | Ghi rõ dataset thật chưa được gửi |
| `third_party/nlohmann/`, `third_party/LICENSE` | nlohmann/json 3.12.0, header-only MIT, kèm để build offline |
| `third_party/README.md` | Nguồn/version/license dependency |
| `README.md`, `docs/*.md`, `docs/evidence/*` | Hướng dẫn, tiến độ, lỗi/interface, kết quả và log thực chạy |
| `reference/original/*` | Giữ nguyên source gốc, kể cả lớp rỗng MinCut và các main cũ, để đối chiếu; không build |
| `reference/AttackGraph.xlsx` | Copy nguyên file lộ trình mới nhất, không sửa trạng thái Excel |

`AttackGraph/include/MinCut.h` và `src/MinCut.cpp` gốc chỉ chứa lớp rỗng/include. Không đưa chúng vào core và không tạo stub trả kết quả giả. Bản nguyên nằm trong reference để thấy đúng mức tiến độ P3.

## API chung đã triển khai

```cpp
Graph graph = loadDataset("data/graph.json");  // File thật cần được bổ sung.
auto before = hackerSimulation(graph, source, target, budget);
Defender defender(graph);
int candidate = defender.suggestCutEdge(source, target);
if (candidate >= 0) graph.blockEdge(candidate);
auto after = hackerSimulation(graph, source, target, budget);
```

| API | Hợp đồng và thay đổi so với cũ |
|---|---|
| `Edge(from,to,weight,relation,blocked,id=-1,capacity=nullopt)` | Giữ 5 tham số đầu. Constructor C++ cũ không có ID: addEdge cấp ID không âm nhỏ nhất còn trống. JSON bắt buộc ID; không tự đánh ID trong loader |
| `getNode(id)` / `getEdge(id)` | Trả con trỏ const, nullptr khi không tồn tại |
| `getNeighbor(id)` | Node ID không tồn tại → invalid_argument; các node đích active không trùng lặp, đúng chiều |
| `getOutGoingEdge(id)` | Trả `vector<const Edge*>`, chỉ cạnh active, giữ các cạnh song song theo ID |
| `getNodes()/getEdges()` | const reference, dùng thống kê/adapter; getEdges gồm cả blocked |
| `blockEdge/unblockEdge/isBlocked` | Theo ID cạnh, ID không tồn tại → invalid_argument |
| `blockEdges(vector<int>)` | Kiểm tra toàn bộ ID trước; nếu có ID sai không patch một phần; ID lặp được phép và patch idempotent |
| `canMove(edgeId, remainingToken)` | true khi cạnh tồn tại, active và token đủ; input thiếu/âm → false |
| `bfs/dfs(start)` | Thứ tự duyệt trên cạnh active; ID thiếu → invalid_argument |
| `validatePath(start,target,edgeIds)` | Xác nhận đầy đủ chiều/thứ tự, blocked và điểm đến; trả valid/cost/message, phân biệt parallel edges |
| `dijkstra(const Graph&,start,target)` | Thay signature ma trận/parent[] cũ; trả PathResult có nodes, edgeIds và totalCost |
| `hackerSimulation(const Graph&,start,target,int64 budget)` | Không tự in; trả AttackResult. Budget âm hoặc node không tồn tại → invalid_argument |
| `printPath/printAttack` | Phần in riêng nhận ostream để CLI/test cùng dùng |
| `Defender(Graph&)` | Thay `Defender(int nodes)`. Xóa nhu cầu Defender::addEdge; thêm cạnh vào Graph chung |
| `Defender::isReachable` | Reachability có hướng S→T, không phải kiểm tra “toàn graph liên thông mạnh” |
| `Defender::suggestCutEdge` | Giữ tên để dễ ghép; trả một edge ID hoặc -1. Không mutation graph; không tối ưu capacity |
| `runSimulation(Graph&,...,optional<vector<int>>)` | Không có danh sách → proposal đơn lẻ; có danh sách → patch thủ công; retest cùng graph/budget, giữ snapshot kết quả Before/After |

Model không đổi sang một framework khác: vẫn Node/Edge, vector và STL. `Neighbors` chuyển từ lưu node đích sang index cạnh để không phải cập nhật cache khi blocked thay đổi. Hàm getter không còn `using namespace std` trong header; code gọi nên dùng `auto` hoặc `std::vector`.

**Lifetime:** con trỏ từ getNode có thể hết hiệu lực khi addNode; con trỏ từ getEdge/getOutGoingEdge có thể hết hiệu lực khi addEdge; cả hai hết hiệu lực khi graph bị gán/hủy. Không lưu chúng lâu dài. Lưu ID rồi truy vấn lại. block/unblock không thêm/xóa phần tử, nên không gây vector reallocation. Defender tham chiếu graph phải có lifetime ngắn hơn graph.

**Ownership:** kiểm tra đề xuất cắt dùng bản sao riêng, không thay đổi graph live. Chỉ bước patch thật tác động graph live. Không có residual edge nào được thêm vào Graph trong bản này.

## Interface đề xuất cho Min-Cut tương lai — chưa triển khai

P3 có thể bàn giao một hàm nhận `const Graph&`, S, T, trả danh sách **ID cạnh gốc**, tổng capacity và trạng thái riêng (`already_disconnected`, `cut_found`, `invalid_input`). Không coi “chưa có capacity” là “cut không tồn tại”. Nhóm cần chốt kiểu trả về trước khi bổ sung vào core.

P1 đã chuẩn bị `getEdges`, `getCapacity`, `blockEdges` và re-test. Khi có Max-Flow, cạnh ngược residual chỉ ở cấu trúc nội bộ của thuật toán. Không patch ID residual, không xóa cạnh khỏi dataset, không chặn theo cặp `(from,to)` khi có cạnh song song.
