# Attack Graph Simulation

C++17 + CMake + JSON. Dataset hiện tại có **18 node, 24 cạnh**, graph có hướng.

## Chạy trong CLion

1. Open folder chứa `CMakeLists.txt`.
2. Chọn toolchain MinGW hoặc Visual Studio.
3. Nếu đang dùng bản cũ: **Tools → CMake → Reset Cache and Reload Project**.
4. Chọn target **AttackGraph**, Build rồi Run; để trống Program arguments.

Mặc định: `PC_LeTan (36) → CustomerDB (3)`, budget `30`. Đường rẻ nhất có cost `28`, còn `2` token.

## Build bằng terminal

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug --parallel 2
ctest --test-dir build -C Debug --output-on-failure
```

Windows MinGW: `build\AttackGraph.exe`. Visual Studio: `build\Debug\AttackGraph.exe`. Linux: `./build/AttackGraph`.

## Tham số

```text
AttackGraph
AttackGraph --demo
AttackGraph --help
AttackGraph <dataset.json> <source> <target> <budget> [--patch <edgeId> ...]
```

Ví dụ trên Windows, chạy từ folder project:

```powershell
.\build\AttackGraph.exe data/graph.json 36 3 30
.\build\AttackGraph.exe data/graph.json 36 3 27
.\build\AttackGraph.exe data/graph.json 36 3 30 --patch 2 6 7
```

Dataset được copy vào `data/` cạnh executable khi build. Chương trình ưu tiên bản copy này, sau đó tìm ở working directory và folder source. Đường dẫn dataset truyền qua CLI được dùng đúng như nhập.

## Kết quả và giới hạn

- `SUCCESS`: cost ≤ budget; `OVER_BUDGET`: có đường nhưng thiếu token; `NO_PATH`: không có đường.
- Sau patch, attacker được cấp lại budget ban đầu. Patch đổi `blocked` trong bộ nhớ, không ghi JSON.
- Auto Defense gọi `minimumSTCut()` rồi block toàn bộ Edge ID trong tập cut. Mặc định: Max-Flow = Min-Cut Capacity = 6, patch `47 48 49`, sau đó `NO_PATH`, reachable `1 → 0`, blocked `0 → 3`.
- Min-Cut Capacity là chi phí phòng thủ của mô hình, khác Attack Cost và Token Budget. Auto Defense vẫn chạy khi attacker ban đầu `OVER_BUDGET` nhưng S–T còn reachable.
- `--patch` giữ chế độ thủ công; không tính Min-Cut, output flow/cut là `N/A`. Kiểm tra toàn bộ ID trước patch, bỏ qua cạnh đã blocked và ID lặp khi báo số cạnh mới chặn.
- S–T đã mất kết nối hoặc source = target: không patch trong cả hai chế độ. Manual ID sai vẫn bị từ chối trước khi thay đổi Graph.
- `runSimulation(Graph&, ...)` cập nhật Graph truyền vào. Để chạy các scenario độc lập, giữ `const Graph initial = loadDataset(...)`, rồi dùng `Graph scenario = initial` cho mỗi lần gọi; hoặc load lại file. Không tự mở lại các cạnh vốn đã blocked trong dataset.
- Mỗi lần chạy CLI load lại dataset, không dùng trạng thái patch lần trước.
- CTest kiểm tra CLI hiện tại. Các báo cáo trong `docs/` là tài liệu lịch sử; repo hiện không có bộ 47 unit test hoặc folder `reference` được nhắc trong báo cáo cũ.
