# Kết quả kiểm thử thực tế

## Môi trường

Linux x86_64, GCC 13.3.0, CMake/CTest 4.4.3, C++17. CMake ban đầu chưa có trong PATH; đã cài công cụ kiểm tra riêng trong workspace. Dependency nlohmann/json 3.12.0 được đóng gói theo project. Chưa chạy trực tiếp Windows/CLion, chưa cross-machine test trên máy của thành viên nhóm.

Các lệnh bên dưới dùng tên `cmake`/`ctest` cho dễ chạy lại; trong môi trường thực đã gọi executable tại `tool-deps/cmake/data/bin/`. Các log run riêng có COMMAND/CWD/EXIT. Root khi kiểm chứng: `/workspace/scratch/b4a55936a706`.

## Bản gốc

```bash
cmake -S original_latest/AttackGraphSimulation-main/AttackGraph -B baseline-build
cmake --build baseline-build -j2
g++ -std=c++17 -I original_latest/AttackGraphSimulation-main/AttackGraph/include original_latest/AttackGraphSimulation-main/AttackGraph/src/*.cpp -o /tmp/original-graph
g++ -std=c++17 original_latest/AttackGraphSimulation-main/player2-hackerteam/p2_hacker_dijkstra.cpp -o /tmp/original-attack
/tmp/original-attack
g++ -std=c++17 original_latest/AttackGraphSimulation-main/Src/main.cpp original_latest/AttackGraphSimulation-main/Src/Defender.cpp -o /tmp/original-defender
/tmp/original-defender
```

| Kiểm tra | Kết quả thực tế |
|---|---|
| Configure CMake Graph gốc | Thành công sau khi có CMake |
| Build Graph gốc | Thất bại tại `Edge.cpp:4`, `*989`; log `baseline-cmake-build.txt` |
| Biên dịch Graph gốc bằng g++ | Thất bại cùng lỗi; `baseline-graph.txt` |
| Demo Attack độc lập | Chạy được: cost 13/budget15 SUCCESS, sau chặn 2→3 cost19 OVER_BUDGET |
| Demo Defender độc lập | Chạy được: cắt ID201, reachable từ true thành false; **không chứng minh Min-Cut** và không bỏ qua lỗi ODR của source |
| Test tự động cũ | Không có file/target test trong source gửi kèm; demo không được tính là test suite |

Các executable có sẵn trong cache ZIP không được dùng để chứng minh source đúng. Hai executable demo trên được biên dịch lại từ source nhận được.

## Bản sửa — Debug và test chức năng

```bash
cmake -S AttackGraphSimulation -B build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug -j2
ctest --test-dir build-debug --output-on-failure
cd AttackGraphSimulation
../build-debug/attackgraph_tests all
```

Kết quả: build đủ `attackgraph_core`, `AttackGraph`, `attackgraph_tests`. **47/47 ca nội bộ đạt**. **9/9 mục CTest đạt**: 6 mục chạy các nhóm của 47 ca, 3 mục kiểm tra CLI. Không cộng thành 56 ca độc lập. Log: `build-debug.txt`, `ctest-debug.txt`, `unit-tests.txt`.

| Nhóm | Số ca nội bộ | Phạm vi |
|---|---:|---|
| Graph G01–G12 | 12 | ID/validation, direction, blocked, BFS/DFS, statistics, path, canMove, copy và pointer lifetime bằng ID |
| Loader L01–L10 | 10 | JSON hợp lệ/lỗi, kiểu dữ liệu, ID, capacity, blocked, atomic load |
| Attack A01–A08 | 8 | Shortest path, budget thiếu/vừa/dư, no path, zero weight, parallel edges, sparse IDs, int64, >40 node |
| Defender D01–D08 | 8 | Reachability, single-edge cut, graph không mutation, already disconnected, nhiều nhánh, capacity limitation |
| Integration I01–I07 | 7 | JSON→Attack→Defender prototype→Patch→retest, reset budget, manual patch, output |
| Regression R01–R02 | 2 | So sánh với Bellman-Ford độc lập trong test; graph rỗng |

R01 dùng seed 20261001, 80 graph nhỏ tạo **chỉ trong test**, mỗi graph có hai trạng thái trước/sau block và 36 cặp source/target: 5.760 phép so sánh path/reachability/cost. Không chỉnh dataset nhóm để test đạt. Bellman-Ford chỉ là oracle trong test, không thay thuật toán Dijkstra sản phẩm.

## Các kết quả quan trọng: expected và actual

| Tình huống | Expected tính tay/hợp đồng | Actual đã kiểm tra |
|---|---|---|
| Demo P2 trước patch | 0→1→2→3, 4+5+4=13 | Path và edge IDs {0,1,2}, cost13 đúng |
| Budget 12 / 13 / 15 | OVER_BUDGET / SUCCESS còn0 / SUCCESS còn2 | Đúng cả ba |
| Đảo chiều 3→0 | Không có đường | NO_PATH dù budget rất lớn |
| Cạnh 0 và chu trình cost0 | Đi được với budget0, không lặp tái tạo path | SUCCESS cost0 |
| Patch ID2 | Đường cũ không hợp lệ, đường mới 0→1→3 cost19 | Cost19, reachable=true |
| Patch ID2, budget15 | Vẫn reachable nhưng vượt budget | OVER_BUDGET, phân biệt NO_PATH |
| Patch ID2, budget20 | Before còn7, After còn1 | Đúng, chứng minh dùng lại budget20 |
| Single-edge prototype trên demo P2 | ID0 ngắt mọi đường từ0 tới3 | Suggest0 không đổi graph; apply xong NO_PATH |
| Patch đúng ID của cạnh song song | Cạnh rẻ bị chặn, cạnh đắt vẫn mở | Path chuyển cost2→9; không chặn nhầm cả cặp node |
| Batch {0,999} | Lỗi, không patch một phần | ID0 vẫn mở |
| S–T đã không reachable | Không đề xuất cạnh bất kỳ | suggest=-1 |
| Hai nhánh edge-disjoint | Không có một cạnh đơn lẻ ngắt hết | suggest=-1; pipeline không giả vờ có Min-Cut |
| Chuỗi capacity100 rồi1 | Weighted cut tối ưu sẽ là cạnh sau; prototype chọn cạnh trước theo thứ tự | D06 xác nhận đúng **giới hạn** của prototype, không ghi “Min-Cut pass” |
| JSON weight0 capacity9 | Attack weight0, capacity9 độc lập | Giữ nguyên hai giá trị |
| Capacity thiếu/null và capacity0 | Unknown khác zero | Kiểm tra đúng, không fallback sang weight |
| Cost2 tỷ +2 tỷ | Tổng4 tỷ, không overflow int32 | int64 totalCost=4.000.000.000 |
| 60 node, ID cách10 | Không bị giới hạn MAX_NODE40 | Cost59, 60 node trên path |
| Graph mẫu P3 | Giữ node0..9, cạnh201..211, cut201 ngắt0→9 | Connectivity trước true/sau false; weights0 chỉ cho fixture connectivity, không suy ra chi phí tấn công |

Các demo được chạy riêng và lưu `demo-auto.txt`, `demo-over-budget.txt`, `demo-reset-budget.txt`.

## Sanitizer

```bash
cmake -S AttackGraphSimulation -B build-sanitize -DCMAKE_BUILD_TYPE=Debug \
  '-DCMAKE_CXX_FLAGS=-fsanitize=address,undefined -fno-omit-frame-pointer' \
  '-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined'
cmake --build build-sanitize -j2
ctest --test-dir build-sanitize --output-on-failure
ASAN_OPTIONS=detect_leaks=0 ctest --test-dir build-sanitize --output-on-failure
```

Lần đầu: 8/9 mục thất bại do **LeakSanitizer không hoạt động dưới ptrace của môi trường**, không ghi nhận là pass; giữ log `ctest-sanitize.txt`. Sau khi tắt riêng leak detection, AddressSanitizer + UndefinedBehaviorSanitizer chạy **9/9 đạt**, log `ctest-sanitize-no-leaks.txt`. **Kiểm tra rò rỉ bộ nhớ chưa thực hiện được.** Việc tắt leak detector không tắt các kiểm tra access/UB còn lại.

## Clean build Release và đóng gói

Đã chạy configure/build **Release thành công**, CTest **9/9 đạt** trong thư mục source khác có dấu cách (`release-check/source with spaces/AttackGraphSimulation`). Không dùng cache hoặc executable từ bản gốc. Log `configure-release.txt`, `build-release.txt`, `ctest-release.txt` ghi đầy đủ lệnh và exit0.

```bash
cmake -S "release-check/source with spaces/AttackGraphSimulation" -B release-check/build -DCMAKE_BUILD_TYPE=Release
cmake --build release-check/build --parallel 2
ctest --test-dir release-check/build --output-on-failure
```

Bản ZIP chỉ chứa source, fixture, dependency header và tài liệu/log; không kèm build cache, object hoặc executable phụ thuộc hệ điều hành.

## Đối chiếu test case trong Excel

| Excel `08_TEST_CASES` | Kết luận |
|---|---|
| T1–T7 | Đã kiểm chứng trên fixture: path, budget, connectivity và blocked |
| T8 Min-Cut graph nhỏ | **Chưa thực hiện với Min-Cut thật**; D02/D04 chỉ là single-edge prototype |
| T9 Min-Cut graph thật | **Chưa kiểm chứng**: không có thuật toán và dataset |
| T10/T11 Before/After | Đạt ở mức prototype/manual patch trên fixture; chưa chứng minh CP2 với Min-Cut |
| T12 Regression | Đã chạy lại CTest cho bản tích hợp này |

## Hai ca nghiệm thu Min-Cut dành cho P3 — chưa có actual

Hai graph dưới đây chỉ là đặc tả test nhỏ, không thay đổi dataset thật. Chưa tạo test PASS cho thuật toán chưa có.

1. Chuỗi S→A→T: capacity cạnh ID10 là100, ID20 là1. Expected max-flow=1, min-cut={20}, tổng capacity1. Sau block20, S không đến T. Prototype hiện tại trả10 nếu thêm10 trước; vì thế không thể kết luận nó tối ưu capacity.
2. Hai nhánh S→A→T và S→B→T: ID1 S→A cap3, ID2 A→T cap2, ID3 S→B cap2, ID4 B→T cap4. Expected max-flow=4, min-cut={2,3}, tổng capacity4. Không có single-edge cut. Sau block2 và3, S không đến T.

Khi P3 bàn giao, cần thêm test capacity0, nhiều cạnh song song/cạnh đối chiều, blocked ban đầu, disconnected và S==T theo hợp đồng. Không tự dùng attack weight làm capacity.

## Giới hạn còn lại

- Chưa có dữ liệu graph thật để đối chiếu node/edge/weight/relation/sơ đồ và chạy scenario thực của nhóm.
- Chưa có Max-Flow/Min-Cut tổng quát; chưa chứng minh weighted optimality, CP2 hoặc MVP đầy đủ.
- Chưa chạy trên Windows/CLion, compiler MSVC/MinGW hoặc máy khác; clean build thư mục khác vẫn là cùng môi trường Linux.
- Validation cú pháp không xác nhận quan hệ Active Directory là đúng nghiệp vụ.
- LeakSanitizer bị giới hạn môi trường như nêu trên.
- Không có GUI/Bridge/Articulation/PPT cuối; các mục này chưa được nghiệm thu trong bản tích hợp prototype.
