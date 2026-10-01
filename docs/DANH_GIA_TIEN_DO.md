# Đánh giá tiến độ dựa trên source và kết quả chạy

Ngày đánh giá: 01/10/2026. Vai trò dùng đúng Excel `00_START` dòng 9–11 và `04_WORKLOAD`; không tự gán tên cá nhân. Người dùng là **Người 1**.

## Căn cứ và tình trạng trước sửa

Nguồn chính: ZIP mới nhất `AttackGraphSimulation-main(3).zip`, Excel `AttackGraph(2).xlsx`. ZIP `(1)`, `(2)`, `(3)` giống byte-for-byte (SHA-256 `c56bd19651b1ac187517cdee3eaf569e28c7cc2b5fdba47719d080ea81246eb1`). Hai Excel `(1)` và `(2)` cũng giống byte-for-byte (SHA-256 `7c4f0a878b5823a42519ff12ffa6f097ae040594b6de333e077ca03676aae952`). Bản tham chiếu giữ source gốc và Excel trong `reference/`.

Không có dataset JSON thật, sơ đồ draw.io hoặc bộ test tự động trong ZIP này. README gốc rỗng; `AttackGraph/src/main.cpp` chỉ `return 0`. `Dijkstra` và `MinCut` trong thư mục Graph là lớp rỗng. Chức năng phải tìm ở hai folder riêng của P2/P3.

Excel đánh dấu P1 ngày 1–2 “Xong”, ngày 3 “Đang làm”; P2 ngày 1 “Xong”; nhiều mục khác “Chưa làm”. Thực tế P2 đã có Dijkstra/path/budget và P3 đã có BFS/blocked/single-edge cut prototype. Ngược lại, Graph gốc chưa build được. Không dùng nhãn Excel làm bằng chứng hoàn thành.

| Người phụ trách | Nhiệm vụ | Bằng chứng trong code nhận được | Trạng thái trước sửa | Lỗi/thiếu sót | Phụ thuộc |
|---|---|---|---|---|---|
| Người 1 | Node/Edge/Graph, CMake | `AttackGraph/include/{Node,Edge,Graph}.h`, `src/*.cpp` | Đã viết khung; **build thực tế thất bại** | `*989` trong Edge.cpp; thiếu kiểm tra dữ liệu; adjacency bỏ qua blocked | Interface chung |
| Người 1 | Dataset/loader/validation | Không có loader hoặc dataset trong ZIP | Chưa có bằng chứng bàn giao | Thiếu JSON và sơ đồ | File dữ liệu thật |
| Người 1 | BFS/DFS, degree, path validation | Không có triển khai | Chưa viết trong bản nhận được | Chưa kiểm chứng | Graph API |
| Người 1 | Patch/integration | `Edge::setBlocked` có; main Graph rỗng | Setter đã viết; chưa có pipeline | Không có edge ID, batch patch, runSimulation | Graph + P2/P3 |
| Người 2 | Dijkstra/path | `player2-hackerteam/p2_hacker_dijkstra.cpp`: minDistance/dijkstra/printPath | Đã viết; demo độc lập **đã chạy**, cost 13 rồi 19 | Mảng tối đa 40, ID phải là index, INF=1e9, globals riêng | Adapter Graph |
| Người 2 | Budget | `hackerSimulation`, so sánh `minCost <= budget` | Đã viết; demo budget 15 **đã chạy** | Chưa test đủ trường hợp bằng ngân sách, ID sai, chi phí lớn; chỉ in kết quả | Kết quả path, patch |
| Người 3 | Connectivity và blocked | `Src/Defender.cpp`: BFS | Đã viết; demo **đã chạy nhưng có lỗi định nghĩa trùng** | BFS index trực tiếp; graph riêng; hai định nghĩa Defender khác nhau | Adapter Graph |
| Người 3 | Đề xuất cắt một cạnh | `suggestCutEdge`: thử block từng cạnh, gọi BFS | Đã viết; demo cắt ID 201 **đã chạy** | Không tối ưu capacity; sai đề xuất khi S–T vốn đã mất kết nối | Connectivity |
| Người 3 | Max-Flow/Minimum S-T Cut | Không có residual graph, augmenting flow hoặc capacity; MinCut.h rỗng | **Chưa triển khai** | Tên/chuỗi output “Min-Cut/optimal” không phải bằng chứng thuật toán | P3 triển khai + nhóm chốt capacity |

“Đã chạy demo” chỉ xác nhận lần chạy đó, không chứng minh mọi input đều đúng. Bản Defender gốc còn vi phạm ODR do `Src/main.cpp` tự định nghĩa cùng tên lớp với `Defender.h` nhưng nội dung khác; link thành công không loại bỏ lỗi này.

## Sau sửa: phần đã có bằng chứng kiểm chứng

| Người phụ trách | Nhiệm vụ | Bằng chứng trong code bàn giao | Trạng thái | Lỗi/thiếu sót còn lại | Phụ thuộc |
|---|---|---|---|---|---|
| Người 1 | Graph API + validation | `Graph.cpp`: addNode/addEdge/getNeighbor/getOutGoingEdge | Đã viết và kiểm chứng G01–G07/G11 | API trả con trỏ đọc tạm; không giữ qua addNode/addEdge | Không |
| Người 1 | JSON loader | `LoadDataset.cpp`; tests L01–L10 | Đã viết và kiểm chứng schema trên fixture | Chưa nạp/so sánh dataset thật | JSON thật |
| Người 1 | BFS/DFS/statistics/path/canMove | `Graph.cpp`; G07–G10/G12 | Đã viết và kiểm chứng | Chưa xác nhận ngữ nghĩa quan hệ trên sơ đồ thật | Dataset/sơ đồ |
| Người 1 | Edge ID, capacity, Patch API | `Edge.h`, `Graph::blockEdges`, G03–G06, L01–L03 | Đã viết và kiểm chứng | Capacity chưa có chính sách nghiệp vụ; giữ null | Nhóm thống nhất capacity |
| Người 2, P1 điều chỉnh interface | Dijkstra/Budget/path | `Dijkstra.cpp`, A01–A08, R01 | Đã tích hợp và kiểm chứng | Chưa test trên dataset thật của nhóm | Dataset |
| Người 3, P1 điều chỉnh interface | Connectivity + single-edge cut | `Defender.cpp`, D01–D08 | Đã tích hợp và kiểm chứng đúng phạm vi prototype | Không có Min-Cut nhiều cạnh/weighted Min-Cut | P3 triển khai |
| Người 1 | Before/Defender/Patch/After/CLI | `Simulation.cpp`, `main.cpp`, I01–I07 | Đã chạy với module thật hiện có trên fixture | Luồng dùng single-edge proposal hoặc patch thủ công, chưa phải pipeline Min-Cut đầy đủ | P3 + dataset |
| Cả nhóm | CP1/CP2/CP3 | Log trong `docs/evidence/` | **Chưa xác nhận checkpoint đầy đủ** | CP1 thiếu dataset thật; CP2 thiếu Min-Cut; CP3 thiếu các phần đó và report/PPT cuối | Nhóm |

Không dùng mock thuật toán. Fixture là dữ liệu nhỏ để kiểm tra; thuật toán Dijkstra và single-edge-cut đều là code thật đã điều chỉnh từ source nhóm. Batch patch thủ công không được ghi là kết quả Min-Cut.

## Người 1 đã đến ngày nào?

Phải đánh dấu theo nhiệm vụ, không lấy ngày lớn nhất để suy ra mọi ngày trước đã xong. Excel có `03_WEEK3` kéo dài ngày 16–28 dù tiêu đề là tuần 3; báo cáo giữ **số ngày trong file**, không tự rút còn 21 ngày.

| Ngày trong Excel | Nhiệm vụ Người 1 | Kết luận bàn giao |
|---|---|---|
| 1 | Model/Relation/interface; rà Dataset | Model/interface đã chuẩn hóa trong code; rà dataset chưa làm được vì thiếu file |
| 2 | C++17/CMake/Node/Edge/Graph | Đã sửa và build được |
| 3 | Hoàn thiện Graph API/adjacency | Đã hoàn thành và kiểm chứng |
| 4 | Load dataset thật | Loader hoàn thành; **nạp dataset thật còn chờ file** |
| 5 | Validate 37–38/~55 | Validation hoàn thành về code; **không xác nhận số lượng/ngữ nghĩa dataset thật** |
| 6 | BFS/DFS/degree/statistics | Đã hoàn thành và kiểm chứng trên fixture |
| 7 | Graph + Hacker + Connectivity | Đã tích hợp, chạy và test trên fixture; CP1 trên graph thật chưa đạt |
| 8 | Edge capacity/blocked/Patch interface | Đã hoàn thành code; capacity là trường tùy chọn, chưa tự định nghĩa nghiệp vụ |
| 9 | blockEdge/unblockEdge/isBlocked/test | Đã hoàn thành và kiểm chứng; thêm batch patch kiểm tra ID trước khi đổi trạng thái |
| 10 | Chạy Min-Cut dataset thật/edge ID | **Chờ P3 Max-Flow/Min-Cut, JSON thật và capacity** |
| 11 | Nối Graph/Hacker/Defender/Patch | Đã nối prototype hiện có; chưa thay thế yêu cầu Min-Cut |
| 12 | runSimulation/pipeline | runSimulation đã chạy trước/sau với budget reset; **ngày 12 đầy đủ còn bị chặn** |
| 13 | CLI/sửa integration bug | Đã có CLI rõ trạng thái, ID, cost, budget, thống kê và lỗi input |
| 14 | Review Graph/Patch/Integration | Đã review và test phạm vi hiện có; cần review lại khi P3 bàn giao Min-Cut |
| 15 | Final integration/dataset | Chưa đạt checkpoint tuần 2 |
| 16 | Refactor Graph/CMake/Patch | Phần độc lập đã làm; không kết luận core toàn nhóm đã ổn định |
| 17 | Report Graph/Dataset/Architecture | Có draft kỹ thuật trong bộ docs; mục dataset thật còn thiếu dữ liệu |
| 18–19 | Validation/full integration tests | Đã test module/prototype; dataset thật và toàn hệ thống có Min-Cut chưa kiểm chứng |
| 20 | README/CMake guide | Đã hoàn thành hướng dẫn; Windows/CLion chưa chạy trực tiếp |
| 21 | Report cuối/hình Graph/Architecture | Chưa có report/PPT cuối hoặc hình graph thật |
| 22–23 | Clean build/package | Đã thực hiện cho bản tích hợp prototype bàn giao này; không phải release candidate đầy đủ của nhóm |
| 24 | Build máy khác | Chưa thực hiện trên máy Windows/CLion của nhóm |
| 25–28 | Fix cuối/buffer/final release | Chưa tuyên bố hoàn thành; phụ thuộc MVP đầy đủ |

**Mốc cao nhất đang chạy được:** Load JSON fixture → Attack trước patch → Connectivity + single-edge proposal thật của P3 → Patch trên cùng Graph → Attack sau patch → so sánh. Đây là phần tích hợp trước/sau đã kiểm chứng, không phải `Minimum S-T Cut → Patch` hoàn chỉnh.

## Việc tiếp theo theo người

### Người 1

1. Nhận dataset JSON thật và sơ đồ tương ứng. Nạp bằng loader, đối chiếu ID, chiều, weight, relation, số node/edge thực; giữ nguyên dữ liệu nguồn. Nếu schema khác, bổ sung adapter có mô tả mapping thay vì đổi dữ liệu âm thầm.
2. Cùng P3 thống nhất ý nghĩa/đơn vị capacity. Chốt ID cạnh rõ ràng nếu dataset gốc chưa có ID; không đánh lại các ID đã có.
3. Khi P3 trả `edgeIds` của Min-Cut, truyền trực tiếp vào `Graph::blockEdges`, chạy lại `hackerSimulation` với budget ban đầu và kiểm tra reachability. Không patch bản sao rồi re-test bản gốc.
4. Chạy graph thật để đóng ngày 4–5, 7; sau đó ngày 10–12/15. Tiếp tục scenario/report và kiểm tra CLion Windows ngày 24.

### Người 2

1. Review bản chuyển interface: `dijkstra(const Graph&,...)`, `PathResult`, `AttackResult`, int64 cost/budget, path theo edge IDs. Logic chọn đỉnh nhỏ nhất và relax vẫn giữ cách của prototype.
2. Bổ sung 5–10 scenario trên dataset thật: source/target khác nhau, budget thiếu/vừa/dư, đường vòng sau patch; ghi expected bằng tay và actual.
3. Nếu nhóm muốn mô phỏng đi từng bước thay vì đánh giá cả đường, bổ sung session/token riêng sử dụng `canMove`. Bản hiện tại đánh giá đủ budget cho cả đường và chỉ tính remainingToken khi SUCCESS; chưa mô phỏng tiêu hao từng bước thất bại giữa đường.
4. Viết phần giải thích độ phức tạp Dijkstra O(V²+E), budget và kết quả thực nghiệm.

### Người 3

1. Triển khai Max-Flow và Minimum S-T Cut thật trên graph nhỏ; có mạng dư và cạnh ngược, không chỉ lặp thử một cạnh.
2. Nhận `const Graph&`, bỏ các cạnh blocked khỏi bài toán, giữ ID gốc khi ánh xạ node sang index/mạng dư. Xử lý cạnh song song và cạnh ngược có sẵn.
3. Từ chối weighted Min-Cut nếu capacity chưa xác định; không dùng weight thay thế. Quy định xử lý S==T và S–T vốn không reachable rõ ràng.
4. Test ít nhất hai graph tính tay trong `KET_QUA_KIEM_THU.md`, xác nhận tổng capacity của cut bằng max-flow; sau patch S không tới T. Bàn giao edge ID và tổng capacity để Người 1 nối vào pipeline.
5. Cập nhật report/output: loại bỏ phát biểu “tối ưu chi phí” cho thuật toán thử cắt một cạnh. Prototype hiện tại chỉ chứng minh một cạnh có thể ngắt cặp S–T, không phải thuật toán bridge tổng quát trên đồ thị vô hướng.
