# Attack Graph Simulation

C++17 + CMake + JSON. Graph có hướng; dataset được load ở runtime theo schema trong `docs/DATASET_SCHEMA.md`. Số node, cạnh, ID và tên do JSON quyết định.

## Chạy trong CLion

1. Open folder chứa `CMakeLists.txt`.
2. Chọn toolchain MinGW hoặc Visual Studio.
3. Nếu đang dùng bản cũ: **Tools → CMake → Reset Cache and Reload Project**.
4. Chọn target **AttackGraph**, Build rồi Run; để trống Program arguments.

Chạy không tham số: chương trình load `data/graph.json`, liệt kê ENTRY, TARGET và tất cả node, rồi yêu cầu nhập Source ID, Target ID, Token Budget. Có thể chọn bất kỳ node tồn tại, kể cả ENDPOINT. Nhập sai được yêu cầu nhập lại; hết input thì báo lỗi.

Trong CLion, để trống Program arguments để chọn tương tác. Hoặc nhập `<dataset.json> <source> <target> <budget> [--patch <edgeId> ...]` theo ID của dataset vừa nhập. Working directory có thể đặt là folder project; đường dẫn CLI tương đối được tính từ working directory.

## Build bằng terminal

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug --parallel 2
```

Windows MinGW: `build\AttackGraph.exe`. Visual Studio: `build\Debug\AttackGraph.exe`. Linux: `./build/AttackGraph`.

## Tham số

```text
AttackGraph
AttackGraph --demo [dataset.json]
AttackGraph <dataset.json>
AttackGraph --help
AttackGraph <dataset.json> <source> <target> <budget> [--patch <edgeId> ...]
```

Ví dụ trên Windows:

```powershell
.\build\AttackGraph.exe
.\build\AttackGraph.exe "D:/datasets/company.json"
.\build\AttackGraph.exe "D:/datasets/company.json" <sourceID> <targetID> <budget>
```

`--demo [dataset.json]` không hỏi input: chọn ENTRY đầu tiên và TARGET đầu tiên theo thứ tự JSON, budget 0. Đây là giá trị demo cố định cho token, không phải giả định về mạng hoặc khả năng tấn công. Thiếu ENTRY/TARGET thì báo lỗi; dùng interactive hoặc CLI để chọn node khác.

Thứ tự tìm file khi không truyền đường dẫn: JSON gốc tại project → `data/graph.json` cạnh executable → `graph.json` cạnh executable → `data/graph.json` trong working directory. Đường dẫn được in tuyệt đối trước khi load. File được chọn sai schema sẽ báo lỗi, không fallback sang file khác. Trong môi trường phát triển, JSON gốc luôn ưu tiên hơn bản cũ trong build.

CMake không copy dataset sau build nữa. Khi đóng gói, tự đặt JSON trong `data/` cạnh `.exe` hoặc truyền đường dẫn cụ thể. Nếu chạy bản đóng gói trên máy vẫn có folder source gốc, truyền đường dẫn cụ thể để ưu tiên file đóng gói. Thay JSON và chạy lại là nhận dữ liệu mới, không rebuild. Các bản copy cũ và dataset người dùng không bị xóa.

Project không sử dụng thư mục `tests/`, CTest hoặc Python. Phần tìm file và chọn tham số được gộp vào `main.cpp`; không cần thêm `RuntimeInput.h/.cpp`.

## Schema runtime hiện tại

JSON có `nodes` và `edges` là array. Mỗi node cần `id`, `name`, `type`, `assets`; mỗi edge cần `id`, `from`, `to`, `weight`, `capacity`, `relation`, và có thể có `blocked` (mặc định false). ID, assets và weight/capacity là số nguyên không âm trong miền `int`, ID duy nhất, endpoint phải tồn tại. Node type: ENTRY, ENDPOINT, IDENTITY, CRITICAL_SYSTEM, TARGET. Relation: HasSession, AdminTo, MemberOf, AccessTo. Name không được rỗng/chỉ khoảng trắng. Các thuật toán vẫn phân biệt weight (attack) và capacity (defense); assets không dùng để tính flow. Loader từ chối key trùng, trường lạ ở root/node/edge và dữ liệu thừa sau JSON; lỗi bản ghi ghi rõ nodes[i]/edges[i]. Capacity bắt buộc, không nhận null. Metadata là object tùy chọn và không tham gia thuật toán. Thiếu ENTRY/TARGET hoặc TARGET không reachable là warning, không tự sửa dữ liệu.

## Kết quả và giới hạn

- `SUCCESS`: cost ≤ budget; `OVER_BUDGET`: có đường nhưng thiếu token; `NO_PATH`: không có đường.
- Sau patch, attacker được cấp lại budget ban đầu. Patch đổi `blocked` trong bộ nhớ, không ghi JSON.
- Auto Defense gọi `minimumSTCut()` rồi block toàn bộ Edge ID trong tập cut, dựa trên dataset và Source/Target được chọn.
- Min-Cut Capacity là chi phí phòng thủ của mô hình, khác Attack Cost và Token Budget. Auto Defense vẫn chạy khi attacker ban đầu `OVER_BUDGET` nhưng S–T còn reachable.
- `--patch` giữ chế độ thủ công; không tính Min-Cut, output flow/cut là `N/A`. Kiểm tra toàn bộ ID trước patch, bỏ qua cạnh đã blocked và ID lặp khi báo số cạnh mới chặn.
- S–T đã mất kết nối hoặc source = target: không patch trong cả hai chế độ. Manual ID sai vẫn bị từ chối trước khi thay đổi Graph.
- `runSimulation(Graph&, ...)` cập nhật Graph truyền vào. Để chạy các scenario độc lập, giữ `const Graph initial = loadDataset(...)`, rồi dùng `Graph scenario = initial` cho mỗi lần gọi; hoặc load lại file. Không tự mở lại các cạnh vốn đã blocked trong dataset.
- Mỗi lần chạy CLI load lại dataset, không dùng trạng thái patch lần trước.
- Các báo cáo trong `docs/` là tài liệu lịch sử; repo hiện không có thư mục test hoặc reference được nhắc trong báo cáo cũ.


## Qt GUI — bước đầu của Phase 3

GUI hiện có giao diện nền tối với ba panel, Import JSON qua loader Core, Explorer theo 5 loại node, tìm theo ID/name, chọn Source/Target và nhập budget trong miền int64_t. Chọn node rồi dùng `Use as Source` / `Use as Target`, hoặc chọn trực tiếp từ dropdown.

Bước này chưa vẽ graph hoặc chạy mô phỏng. Các nút Attack/Defend/Re-Attack/Reset đang disabled; bảng Before/After hiển thị `--` cho kết quả chưa có. Không hiển thị kết quả giả. Import lỗi giữ dataset đang mở; đường dẫn Unicode được đọc qua QFile rồi truyền stream vào loader Core. Dataset rỗng vẫn import được và hiển thị warning, nhưng không có node để chọn.

Chỉ thêm `gui/main.cpp` và `gui/MainWindow.h/.cpp`. GraphView sẽ được thêm ở bước vẽ graph. Core và CLI không phụ thuộc Qt.

### CLion Windows / MinGW

1. Cài Qt 6 Widgets bản **MinGW 64-bit** và compiler tương ứng với bộ Qt. Không chọn bộ MSVC khi build bằng MinGW.
2. Trong **Settings → Build, Execution, Deployment → Toolchains**, chọn compiler MinGW đi kèm bộ Qt.
3. Trong **CMake → CMake options**, thêm:

```text
-DBUILD_GUI=ON -DCMAKE_PREFIX_PATH=C:/Qt/<version>/mingw_64
```

Thay `<version>` bằng phiên bản đã cài, hoặc dùng đường dẫn Qt thực tế của bạn. Nếu đường dẫn có khoảng trắng, đặt giá trị trong dấu ngoặc kép. Ví dụ Qt 6.8 dùng bộ MinGW-w64 13.1 tương ứng.

4. Reset Cache and Reload Project, Build target **AttackGraphGUI**.
5. Trong Run Configuration chọn executable **AttackGraphGUI**. Thêm thư mục `bin` của Qt và MinGW vào `PATH` của Run Configuration để tìm DLL và plugin Qt khi chạy.
6. Run rồi bấm **Import JSON**, chọn `data/graph.json`. Có thể truyền đường dẫn JSON vào Program arguments để mở khi khởi động.

Nếu thiếu `Qt6Config.cmake`, kiểm tra CMAKE_PREFIX_PATH trỏ đến folder `mingw_64`, không phải folder Tools/MinGW. Nếu báo thiếu DLL hoặc plugin `windows`, kiểm tra đúng bộ Qt và môi trường PATH. Chạy trên máy khác cần DLL/plugin tương ứng; chưa đóng gói bản phát hành trong bước này.

### Build bằng terminal

```sh
cmake -S . -B build-gui -DBUILD_GUI=ON -DCMAKE_PREFIX_PATH="C:/Qt/<version>/mingw_64"
cmake --build build-gui --target AttackGraphGUI --parallel 2
```

Mặc định `BUILD_GUI=OFF`: cách build CLI hiện tại vẫn hoạt động trên máy chưa cài Qt. Bật GUI mà chưa có Qt sẽ báo lỗi cấu hình rõ ràng.
