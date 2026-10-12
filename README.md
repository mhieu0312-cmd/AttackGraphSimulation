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
cmake -S . -B build -DBUILD_GUI=OFF -DCMAKE_BUILD_TYPE=Debug
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


## Giao diện raylib

Bản GUI sử dụng **raylib 6.0**, thay cho Qt. Core C++17 và CLI giữ nguyên. Graph được vẽ theo 5 nhóm từ JSON; không hardcode Source, Target, Edge ID hoặc kết quả. Giao diện nền tối, graph ở giữa, tham số phía trên, kết quả Before/After và các nút phía dưới.

### Chạy bằng CLion trên Windows / MinGW

1. Mở folder chứa `CMakeLists.txt`. Dùng CMake **3.25 trở lên** (CMake đi kèm CLion 2025.2 đáp ứng yêu cầu này).
2. Trong **Settings → Build, Execution, Deployment → Toolchains**, chọn MinGW có cả C và C++ compiler. Không cần compiler đi kèm Qt.
3. Trong **Settings → Build, Execution, Deployment → CMake**, xóa tùy chọn `CMAKE_PREFIX_PATH` trỏ tới Qt cũ và đặt:

```text
-DBUILD_GUI=ON
```

4. Chọn **Tools → CMake → Reset Cache and Reload Project**. Lần cấu hình đầu cần internet: CMake tải source raylib đúng phiên bản và kiểm tra SHA256. Các lần build tiếp theo dùng source đã tải trong build folder.
5. Chọn target / Run Configuration **AttackGraphGUI**, Build rồi Run. `AttackGraph` là chương trình console, không mở GUI.

Không cần tài khoản Qt, cài Qt hoặc cấu hình Qt DLL/plugin. Source raylib được biên dịch thành thư viện tĩnh cùng project. MinGW vẫn có thể yêu cầu các DLL runtime của compiler khi chạy ngoài CLion; trong CLion hãy sử dụng môi trường của toolchain. Nộp source project cùng `CMakeLists.txt`, `include/`, `src/`, `gui/`, `third_party/`, `data/`; không cần nộp `.exe`, `.idea` hoặc `cmake-build-*`. Máy của giảng viên cần compiler C/C++ và CMake, như khi build một project C++ thông thường.

Build bằng terminal MinGW:

```sh
cmake -S . -B build-gui -DBUILD_GUI=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build-gui --target AttackGraphGUI --parallel 2
```

Nếu mạng chặn GitHub/codeload hoặc cần build offline: tải source tag **6.0** từ [raylib](https://github.com/raysan5/raylib/releases/tag/6.0), giải nén và đặt thêm `-DRAYLIB_SOURCE_DIR=C:/libraries/raylib-6.0` (folder chứa `CMakeLists.txt`). Không dùng file `.exe` raylib thay cho source. Chỉ cần CLI thì đặt `-DBUILD_GUI=OFF`, không tải raylib.

Linux cần các gói phát triển OpenGL và X11 của hệ điều hành. Ví dụ Ubuntu: `libgl1-mesa-dev libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev`. Đây là thư viện nền tảng Linux, không cần cài trên Windows.

### Sử dụng

- **Import JSON**: dán đường dẫn vào ô phía trên rồi bấm Import; hỗ trợ Ctrl+A, Ctrl+V, Backspace. Có thể kéo thả file JSON vào cửa sổ hoặc truyền đường dẫn trong Program arguments. Import sai báo lỗi và giữ graph/kết quả hiện tại. Rê chuột lên dòng thông báo để đọc nội dung dài.
- **Source / Target**: bấm ô để mở Explorer, tìm ID hoặc name; danh sách xếp theo 5 loại node và cuộn bằng con lăn. Hoặc chọn node trên graph rồi nhấn **S** / **T**. Bất kỳ node tồn tại đều có thể làm endpoint. Mặc định chọn ENTRY/TARGET đầu tiên theo JSON, nếu thiếu thì dùng node đầu/cuối.
- **Budget**: số nguyên không âm đến `INT64_MAX`. Source, Target hoặc Budget thay đổi sẽ khôi phục graph gốc và xóa kết quả cũ để không trộn hai scenario.
- **Attack**: gọi `hackerSimulation()` (Dijkstra), highlight đường có tổng Weight nhỏ nhất bằng màu cam. `OVER_BUDGET` vẫn hiển thị đường và cost; `NO_PATH` hiển thị cost/token N/A.
- **Defend**: gọi `minimumSTCut()` (Edmonds–Karp), highlight tập cut màu tím, mở danh sách chọn cạnh. Tất cả cạnh cut được chọn mặc định. Có thể bỏ chọn để thử Patch một phần. Capacity là chi phí phòng thủ, khác Weight của tấn công.
- **Apply Patch** trong hộp chọn: block các cạnh đã chọn trong bộ nhớ; cạnh bị block màu đỏ nét đứt. **Review Cut** mở lại hộp nếu đã đóng. Tập Min-Cut và tập đã Patch được hiển thị riêng; Patch một phần có thể chưa ngắt kết nối.
- **Re-Attack**: chạy lại Dijkstra trên graph đã Patch với **budget ban đầu**, cập nhật bảng Before/After. Source = Target hoặc đã mất kết nối thì Defend giữ graph, không tính cut và vẫn cho Re-Attack.
- **Reset**: phục hồi đúng graph từ lần Import, kể cả cạnh vốn bị blocked. Không ghi đè JSON. Các nút chỉ bật khi đúng giai đoạn: Attack → Defend → Apply Patch → Re-Attack → Reset.
- **View Before / Current**: đổi dữ liệu hiển thị, không thay đổi trạng thái mô phỏng. Node giữ nguyên vị trí và camera khi đổi view, Patch hoặc Reset.
- Con lăn để zoom; giữ chuột trái trên node và kéo để đổi vị trí, các cạnh đi theo node. Kéo vùng trống hoặc giữ nút chuột giữa để pan; **Fit** đưa toàn bộ graph vào khung. **Labels** bật Relation / Weight / Capacity thực tế. **Focus** làm mờ cạnh ngoài đường tấn công/cut. Màu node biểu thị loại node; vòng cam đánh dấu node được chọn.

JSON mặc định được tìm ở project → `data/graph.json` cạnh chương trình → `graph.json` cạnh chương trình → `data/graph.json` trong working directory. Không copy JSON tự động vào build folder, nên sửa/thay dataset không cần rebuild. GUI đọc đường dẫn UTF-8 bằng `std::filesystem::u8path`; font Arial trên Windows hoặc DejaVu Sans trên Linux hỗ trợ tiếng Việt. Dataset dùng bộ chữ khác cần font phù hợp; font mặc định raylib là fallback.

### Cấu trúc dễ giải thích

Chỉ có **5 file GUI**, hai class:

- `gui/main.cpp`: tạo cửa sổ, chọn font, tìm dataset, chạy vòng lặp giao diện.
- `gui/SimulationUI.h/.cpp`: giữ graph gốc/graph hiện tại, xử lý các nút và gọi Core, hiển thị kết quả.
- `gui/GraphView.h/.cpp`: chia node vào 5 cột, vẽ cạnh có hướng, chọn node, zoom/pan và highlight. Không chứa Dijkstra hoặc Min-Cut.

Các hàm Attack/Defend/Re-Attack nhận tham số đã chọn trong GUI và lưu kết quả trả về từ Core; Apply Patch dùng vector Edge ID đã chọn. Graph gốc được copy để Reset và xem Before. Hai vector vị trí và cạnh trong GraphView chỉ phục vụ vẽ, vì vậy đổi trạng thái blocked không làm node nhảy vị trí. Cạnh thường là đoạn thẳng có mũi tên, cạnh song song/đối hướng được dịch nhẹ, cạnh tự nối được vẽ bằng các đoạn thẳng quanh node. Kéo node chỉ đổi tọa độ giao diện, không đổi Graph, Weight hay Capacity. Vị trí đã kéo giữ khi Attack, Patch, Before/After và Reset; Import dataset sẽ bố trí lại. Fit căn camera theo vị trí hiện tại, không tự sắp xếp lại node. Tọa độ không được ghi vào JSON.

### Kiểm chứng bản chuyển đổi

Đã build native Linux, cấu hình tải raylib tự động và build cross-compile Windows bằng MinGW-w64 13. Đã chạy cửa sổ raylib thực tế trên màn hình ảo Linux, kiểm tra baseline, Patch toàn bộ/một phần, budget thiếu/tràn, Source = Target, NO_PATH, Import sai giữ trạng thái, dataset ID khác, cạnh capacity 0/parallel, Reset, kéo node ở nhiều mức zoom, pan và vị trí Before/After. Chưa chạy trực tiếp trên Windows/CLion của người dùng; cross-compile không thay thế kiểm chứng runtime trên máy đó. Không thêm thư mục `tests/` hoặc thay Core/dataset chính.
