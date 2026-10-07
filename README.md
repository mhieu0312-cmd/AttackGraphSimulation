# Attack Graph Simulation

Bản tích hợp C++17 cho Graph, Attack và Connectivity/Patch. Đã giữ cách Dijkstra của Người 2 và thuật toán thử cắt một cạnh của Người 3, đồng thời chuyển sang dùng chung `Graph`.

**Chưa có Max-Flow/Minimum S-T Cut tổng quát. Các ZIP được gửi không chứa dataset JSON thật.** Vì vậy, demo mặc định dùng graph mẫu 4 node có sẵn trong source Người 2. Đây là bản tích hợp prototype đã kiểm thử, chưa phải MVP đầy đủ trên mạng thật của nhóm.

## Mở trong CLion trên Windows

1. Giải nén ZIP thành folder `AttackGraphSimulation`.
2. CLion → **Open** → chọn folder có `CMakeLists.txt` ở cấp ngoài cùng. Không mở project bên trong `reference/original`.
3. Trong Settings → Build, Execution, Deployment → Toolchains, chọn bộ C++ có sẵn của CLion, MinGW hoặc Visual Studio; dùng CMake đi kèm CLion.
4. Reload CMake. Chọn target **AttackGraph**, Build rồi Run. Không cần nhập tham số để chạy demo mẫu.
5. Muốn chạy dữ liệu khác: Edit Configurations → Program arguments theo cú pháp bên dưới; Working directory đặt bằng đường dẫn folder project của bạn.
6. Chạy target **attackgraph_tests** với Working directory là folder project và Program arguments `all`, hoặc dùng CTest qua Terminal. Test và demo là hai executable riêng.

Thư viện JSON đã nằm trong `third_party/`, build không cần Internet. Không đưa lại `cmake-build-debug` của máy cũ vào project. Không cần cài Python để build/run.

## Build và test bằng Terminal

Chạy từ folder `AttackGraphSimulation`, với `cmake`/`ctest` nằm trong PATH của terminal/toolchain:

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug --parallel 2
ctest --test-dir build -C Debug --output-on-failure
```

MinGW/Ninja thường đặt executable tại `build/`. Visual Studio đa cấu hình thường đặt tại `build/Debug/`.

```powershell
.\build\AttackGraph.exe
.\build\AttackGraph.exe tests/fixtures/p2_demo.json 0 3 15 --patch 2
.\build\AttackGraph.exe tests/fixtures/p2_demo.json 0 3 20 --patch 2
.\build\attackgraph_tests.exe all
```

Nếu dùng Visual Studio, thay `build\` bằng `build\Debug\` trong các lệnh executable. Trên Linux dùng `./build/AttackGraph` và `./build/attackgraph_tests`. CLion trên Windows chưa được chạy trực tiếp trong lần bàn giao này; xem bằng chứng môi trường Linux trong `docs/KET_QUA_KIEM_THU.md`.

## CLI và kết quả mong đợi

```text
AttackGraph [--demo]
AttackGraph <dataset.json> <source> <target> <budget> [--patch <edgeId> ...]
```

- Không truyền `--patch`: gọi `Defender::suggestCutEdge`, thuật toán thật từ prototype P3, chỉ tìm **một cạnh** có thể ngắt S–T. Không gọi nó là weighted Min-Cut.
- Truyền `--patch`: chặn chính xác các ID yêu cầu, dùng để kiểm tra phần Patch độc lập. Không coi danh sách này là kết quả của Min-Cut.
- `SUCCESS`: có đường và tổng cost ≤ budget. `OVER_BUDGET`: có đường nhưng không đủ token. `NO_PATH`: không có đường, dù tăng budget cũng không giải quyết được.
- Mỗi lần re-test nhận **budget ban đầu**, không lấy số token còn lại của lần trước.
- `runSimulation` thay đổi graph truyền vào bằng tham chiếu. Gọi lại trên graph đó giữ trạng thái blocked hiện tại. Load lại file hoặc copy graph ban đầu để bắt đầu scenario độc lập.
- Patch chỉ đổi `blocked` trong bộ nhớ; không xóa cạnh, không ghi đè JSON trên đĩa.
- Exit code `0`: chạy mô phỏng thành công, kể cả Hacker thất bại. Exit code `1`: tham số/dataset lỗi.

| Demo | Trước patch | Sau patch |
|---|---|---|
| Mặc định: prototype P3 chọn cạnh 0 | Cost 13, budget 15, SUCCESS | NO_PATH |
| `--patch 2`, budget 15 | Cost 13, còn 2 | Cost 19, OVER_BUDGET, vẫn reachable |
| `--patch 2`, budget 20 | Cost 13, còn 7 | Cost 19, SUCCESS, còn 1 |

## File cần đọc

| Đường dẫn | Nội dung |
|---|---|
| `include/`, `src/` | Model và module tích hợp |
| `tests/test_main.cpp` | 47 ca kiểm thử, chia 6 nhóm |
| `tests/fixtures/p2_demo.json` | Graph mẫu P2, giữ node ID/hướng/weight gốc; không phải dataset thật |
| `data/README.md` | Tình trạng dataset thật đang thiếu |
| `docs/DANH_GIA_TIEN_DO.md` | Bằng chứng code, tiến độ 3 người và mốc Người 1 |
| `docs/THAY_DOI_VA_INTERFACE.md` | Lỗi gốc, danh sách sửa và cách gọi API |
| `docs/DATASET_SCHEMA.md` | Định dạng JSON, ID, weight/capacity |
| `docs/KET_QUA_KIEM_THU.md` | Lệnh, kết quả thực chạy, giới hạn |
| `docs/evidence/` | Log build, test và demo |
| `reference/AttackGraph.xlsx` | Excel gốc, giữ nguyên nội dung |
| `reference/original/` | Source gốc để đối chiếu; không thuộc target build |

## Việc cần bàn giao tiếp

Người 1 đã có phần độc lập của ngày 3, 6, 8, 9; tích hợp cơ bản và các phần pipeline/CLI của ngày 7, 11–14 đã chạy trên fixture. **Không thể đánh dấu hoàn thành liên tục đến ngày 9 hoặc đạt CP1/CP2**, vì ngày 4–5 còn thiếu dataset thật; ngày 10–12/15 còn chờ Max-Flow/Min-Cut của Người 3 và quy tắc capacity được cả nhóm thống nhất. Chi tiết từng ngày nằm trong báo cáo tiến độ.
