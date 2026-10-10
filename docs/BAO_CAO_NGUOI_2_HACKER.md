# BÁO CÁO KỸ THUẬT NGƯỜI 2 (P2)
## PHÂN HỆ RED TEAM: MÔ HÌNH HÓA VÀ MÔ PHỎNG TẤN CÔNG BẰNG DIJKSTRA & TOKEN BUDGET

**Đề tài:** Mô hình hóa và phân tích lộ trình tấn công/phòng thủ mạng nội bộ bằng Attack Graph Simulation  
**Hệ thống mục tiêu:** Mạng nội bộ doanh nghiệp HAH Solutions  
**Thành viên phụ trách (P2):** Red Team, Thuật toán Dijkstra, Quản lý Token Budget, Hacker Simulation, Scenario Testing  

---

## 1. Bối cảnh và Bài toán đặt ra cho Red Team

### 1.1 Bối cảnh hệ thống mạng HAH Solutions
Hệ thống mạng nội bộ của HAH Solutions bao gồm nhiều tầng thành phần:
- **Workstation / Máy trạm:** `PC_LeTan` (Initial Foothold - Lễ tân), `PC_HR1`, `PC_KeToan1`, `PC_Dev1`.
- **User Identity / Nhóm tài khoản:** `User_LienHR`, `User_KeToanTruong`, `UserLeadDev`, `Group_HR`, `Group_IT`, `GroupKeToan`.
- **Critical System / Server nội bộ:** `Server_Internal`, `Server_App`, `Server_File`, `Server_DB`.
- **Critical Assets (Mục tiêu cần bảo vệ - Target):**
  1. `CustomerDB` (Cơ sở dữ liệu khách hàng - ID 3)
  2. `AdminDB` / `Financial Database` (Cơ sở dữ liệu tài chính/quản trị - ID 2)
  3. `SeverDC` (Domain Controller - ID 1)
  4. `BackupServer` (Hệ thống sao lưu dự phòng - ID 13)

### 1.2 Bài toán của Attacker (Người 2 - P2)
Giả định kẻ tấn công đã chiếm được quyền điều khiển máy trạm ban đầu **`PC_LeTan` (ID 36)**. Từ bàn đạp này, kẻ tấn công tìm cách leo thang đặc quyền và di chuyển ngang (Lateral Movement) qua các quan hệ:
- `HasSession`: Phiên làm việc còn lưu của người dùng trên máy trạm.
- `MemberOf`: Thành viên của các nhóm người dùng đặc quyền.
- `AccessTo`: Quyền truy cập dịch vụ, máy chủ hoặc cơ sở dữ liệu.
- `AdminTo`: Quyền quản trị trực tiếp lên hệ thống máy chủ.

**Nhiệm vụ của P2:**
1. Xác định Attacker có đường đi từ điểm bắt đầu (`Source`) tới mục tiêu chỉ định (`Target`) hay không?
2. Nếu có đường đi, tìm đường có **tổng chi phí tấn công nhỏ nhất** ($TotalCost_{min}$)?
3. So sánh chi phí tối thiểu với **Ngân sách Token ban đầu** (`Budget`): Đủ token (`SUCCESS`), thiếu token (`OVER_BUDGET`), hay không có đường (`NO_PATH`)?
4. Đánh giá điều kiện di chuyển qua từng cạnh (`canMove`) dựa trên trạng thái cạnh và token còn lại.
5. Khi Defender thực hiện khắc phục (chặn/vá cạnh bằng Min-Cut hoặc thủ công), chạy lại mô phỏng (**Re-test**) để đánh giá sự thay đổi của đường tấn công (chi phí tăng lên do phải đi đường vòng hoặc bị ngắt hoàn toàn).

---

## 2. Thiết kế Thuật toán & Cấu trúc Dữ liệu

### 2.1 Tại sao chọn Thuật toán Dijkstra?
- **Đặc trưng bài toán:** Trọng số cạnh $c_{atk}$ (`weight`) đại diện cho chi phí, thời gian và độ phức tạp mà Attacker phải bỏ ra để vượt qua một quan hệ bảo mật. Trọng số này **luôn không âm** ($weight \ge 0$).
- **Tính tối ưu:** Dijkstra đảm bảo tìm được đường đi ngắn nhất (chi phí thấp nhất) từ một đỉnh nguồn tới mọi đỉnh khác trong đồ thị có hướng có trọng số không âm.
- **Tách biệt mô hình Đa trọng số (Dual-Weight):**
  - $c_{atk}$ (`weight`): Chi phí Attacker bỏ ra (dùng trong thuật toán của P2).
  - $c_{def}$ (`capacity`): Chi phí vận hành/khắc phục mà Defender phải chịu (dùng trong thuật toán Min-Cut của P3).
  - P2 **hoàn toàn độc lập** với `capacity`, chỉ tập trung vào hành vi tối ưu hóa của Attacker dựa trên `weight`.

### 2.2 Cấu trúc dữ liệu cốt lõi của P2

```cpp
// 1. Kết quả đường đi tìm được bởi Dijkstra
struct PathResult {
    bool reachable = false;     // Có đến được Target hay không
    int64_t totalCost = 0;      // Tổng chi phí (weight) nhỏ nhất
    vector<int> nodes;          // Chuỗi ID các node trên đường đi (Source -> ... -> Target)
    vector<int> edgeIds;        // Chuỗi ID các cạnh được sử dụng
};

// 2. Trạng thái kết quả tấn công
enum class AttackStatus {
    SUCCESS,        // Tiếp cận Target thành công (totalCost <= budget)
    OVER_BUDGET,    // Có đường tới Target nhưng thiếu Token (totalCost > budget)
    NO_PATH         // Đồ thị mất kết nối, không có đường đi tới Target
};

// 3. Kết quả mô phỏng Hacker đầy đủ
struct AttackResult {
    PathResult path;                            // Chi tiết đường đi
    AttackStatus status = AttackStatus::NO_PATH; // Trạng thái
    int64_t initialBudget = 0;                  // Ngân sách Token ban đầu
    optional<int64_t> remainingToken;           // Token còn lại = budget - totalCost (khi SUCCESS)
};
```

### 2.3 Chi tiết các bước thực thi trong `dijkstra()`
1. **Kiểm tra đầu vào (Precondition Check):** Xác minh `start` và `target` có tồn tại trong `Graph`. Nếu không có, ném ngoại lệ `invalid_argument`.
2. **Trường hợp biên (Source == Target):** Nếu hacker xuất phát ngay tại mục tiêu, chi phí bằng $0$, `nodes = {start}`, `edgeIds = {}`, `reachable = true`.
3. **Ánh xạ đỉnh linh hoạt (Index Mapping):** Sử dụng `unordered_map<int, size_t> index` để ánh xạ từ ID thực tế của Node trong dataset sang chỉ số $0 \dots (V-1)$. Giúp thuật toán hoạt động an toàn với ID không liên tục hoặc ID thưa (Sparse IDs).
4. **Khởi tạo (Initialization):**
   - Mảng `distance[V]` khởi tạo bằng $\infty$ (`numeric_limits<int64_t>::max()`). Gán `distance[start] = 0`.
   - Mảng `visited[V]` khởi tạo bằng `false`.
   - Mảng `parentEdge[V]` khởi tạo bằng `-1` để lưu ID cạnh dẫn đến từng node.
5. **Vòng lặp chính (Relaxation Loop):**
   - Tìm đỉnh $u$ chưa duyệt có `distance[u]` nhỏ nhất bằng hàm `minDistance()`.
   - Nếu không còn đỉnh nào đến được hoặc đã chạm tới `target`: Dừng sớm (Early exit).
   - Đánh dấu `visited[u] = true`.
   - Lấy danh sách cạnh xuất phát còn hoạt động qua `graph.getOutGoingEdge(u)` (đã tự động lọc bỏ các cạnh bị Defender chặn `!isBlocked()`).
   - Với mỗi cạnh $(u, v)$ có trọng số $w$:
     - Kiểm tra an toàn chống tràn số nguyên 64-bit (`inf - w`).
     - Nếu $distance[u] + w < distance[v]$, cập nhật:
       $$distance[v] = distance[u] + w$$
       $$parentEdge[v] = edge \to getID()$$
6. **Truy vết đường đi (Path Reconstruction):**
   - Nếu `distance[target] == inf`: Trả về `PathResult{}` (`reachable = false`).
   - Ngược lại, truy vết ngược từ `target` về `start` bằng cách lần theo `parentEdge`, sau đó đảo ngược vector để thu được đúng thứ tự `start -> ... -> target`.

### 2.4 Điều kiện di chuyển từng bước: Hàm `canMove()`
Đáp ứng yêu cầu mô phỏng kiểm soát từng chặng:
```cpp
bool canMove(const Graph& graph, int edgeId, int64_t remainingToken) {
    if (remainingToken < 0) return false;
    const Edge* edge = graph.getEdge(edgeId);
    if (edge == nullptr || edge->isBlocked()) return false;
    return remainingToken >= edge->getWeight();
}
```
- Trả về `true` khi và chỉ khi: Cạnh tồn tại, chưa bị chặn (`!isBlocked`), và số token còn lại đủ chi trả trọng số cạnh (`remainingToken >= weight`).

### 2.5 Đánh giá Ngân sách: Hàm `hackerSimulation()`
- Kiểm tra `budget >= 0` (ngân sách không được âm).
- Gọi `dijkstra(graph, start, target)`.
- Phân loại kết quả:
  - Nếu `!path.reachable` $\rightarrow$ `status = AttackStatus::NO_PATH`.
  - Nếu `path.totalCost > budget` $\rightarrow$ `status = AttackStatus::OVER_BUDGET`.
  - Nếu `path.totalCost <= budget` $\rightarrow$ `status = AttackStatus::SUCCESS`, và `remainingToken = budget - path.totalCost`.

---

## 3. Phân tích Độ phức tạp Thuật toán

| Tiêu chí | Cài đặt hiện tại (Mảng) | Cài đặt Min-Heap (Ưu tiên) | Ghi chú trong đồ án |
|---|---|---|---|
| **Độ phức tạp Thời gian** | $O(V^2 + E)$ | $O((V + E) \log V)$ | Với $V = 18, E = 24$, cả hai cách chạy dưới 0.1 ms. Cài đặt mảng trực quan, dễ hiểu, không lỗi con trỏ. |
| **Độ phức tạp Không gian** | $O(V)$ | $O(V + E)$ | Chỉ sử dụng các vector kích thước $V$ (`distance`, `visited`, `parentEdge`). |
| **Tính đúng đắn** | Tối ưu toàn cục | Tối ưu toàn cục | Do mọi trọng số $weight \ge 0$, thuật toán luôn hội tụ về đường đi rẻ nhất. |

---

## 4. Kết quả Thực nghiệm: Bộ 10 Kịch bản trên Dataset thật (`data/graph.json`)

Dataset chính thức gồm **18 node, 24 edge**, điểm khởi đầu xâm nhập là `PC_LeTan (ID 36)`.

| ID | Tình huống / Kịch bản | Source $\to$ Target | Budget | Lộ trình tấn công (Node IDs & Tên) | Cạnh sử dụng (Edge IDs) | Cost | Trạng thái | Token còn lại | Ý nghĩa thực tiễn |
|:---:|---|:---:|:---:|:---|:---:|:---:|:---:|:---:|---|
| **SC-01** | Mục tiêu CustomerDB (Mặc định) | 36 $\to$ 3 | 30 | `PC_LeTan (36)` $\to$ `PC_Dev1 (32)` $\to$ `UserLeadDev (20)` $\to$ `Server_App (7)` $\to$ `CustomerDB (3)` | 48, 43, 25, 6 | 28 | **SUCCESS** | 2 | Đường ngắn nhất qua nhóm Lập trình viên để đánh cắp CSDL khách hàng. |
| **SC-02** | CustomerDB thiếu ngân sách | 36 $\to$ 3 | 27 | `PC_LeTan (36)` $\to$ `PC_Dev1 (32)` $\to$ `UserLeadDev (20)` $\to$ `Server_App (7)` $\to$ `CustomerDB (3)` | 48, 43, 25, 6 | 28 | **OVER_BUDGET** | N/A | Hacker tìm ra đường đi nhưng không đủ tài nguyên thâm nhập. |
| **SC-03** | Đánh sập Domain Controller | 36 $\to$ 1 | 35 | `PC_LeTan (36)` $\to$ `PC_HR1 (26)` $\to$ `User_LienHR (14)` $\to$ `Server_Internal (4)` $\to$ `SeverDC (1)` | 47, 37, 12, 1 | 29 | **SUCCESS** | 6 | Lộ trình khai thác qua phòng Nhân sự để leo thang lên Domain Controller. |
| **SC-04** | Đánh cắp CSDL Tài chính AdminDB | 36 $\to$ 2 | 35 | `PC_LeTan (36)` $\to$ `PC_KeToan1 (28)` $\to$ `User_KeToanTruong (15)` $\to$ `Server_DB (9)` $\to$ `AdminDB (2)` | 49, 38, 14, 8 | 31 | **SUCCESS** | 4 | Lộ trình khai thác qua máy phòng Kế toán để đánh cắp dữ liệu tài chính. |
| **SC-05** | Tấn công Máy chủ Backup | 36 $\to$ 13 | 35 | `PC_LeTan (36)` $\to$ `PC_Dev1 (32)` $\to$ `UserLeadDev (20)` $\to$ `Server_File (6)` $\to$ `BackupServer (13)` | 48, 43, 26, 4 | 34 | **SUCCESS** | 1 | Chi phí cao nhất (34 token), chỉ vừa đủ ngân sách 35. |
| **SC-06** | Defender vá Edge 48 (Đường vòng) | 36 $\to$ 3 | 35 | `PC_LeTan (36)` $\to$ `PC_KeToan1 (28)` $\to$ `User_KeToanTruong (15)` $\to$ `Server_DB (9)` $\to$ `CustomerDB (3)` | 49, 38, 14, 7 | 29 | **SUCCESS** | 6 | Khi cạnh 48 bị chặn, Hacker chuyển hướng sang nhánh Kế toán, chi phí tăng từ 28 lên 29. |
| **SC-07** | Đổi trạng thái sau khi vá Edge 48 | 36 $\to$ 3 | 28 | `PC_LeTan (36)` $\to$ `PC_KeToan1 (28)` $\to$ `User_KeToanTruong (15)` $\to$ `Server_DB (9)` $\to$ `CustomerDB (3)` | 49, 38, 14, 7 | 29 | **OVER_BUDGET** | N/A | Trước vá: Budget 28 thành công (Cost 28). Sau vá: Đường mới Cost 29 > 28 $\rightarrow$ Thất bại. |
| **SC-08** | Defender áp dụng Minimum S-T Cut | 36 $\to$ 3 | 50 | Không còn đường đi | None | $\infty$ | **NO_PATH** | N/A | Defender cắt toàn bộ lát cắt cực tiểu `{47, 48, 49}`, triệt tiêu mọi khả năng tấn công. |
| **SC-09** | Tấn công từ Workstation nội bộ | 32 $\to$ 13 | 30 | `PC_Dev1 (32)` $\to$ `UserLeadDev (20)` $\to$ `Server_File (6)` $\to$ `BackupServer (13)` | 43, 26, 4 | 24 | **SUCCESS** | 6 | Mô phỏng trường hợp Insider Threat xuất phát ngay từ máy phòng Dev. |
| **SC-10** | Kiểm tra điều kiện canMove | N/A | N/A | Edge 48: Weight = 10, Token = 15/10 $\rightarrow$ `true`; Token = 9 $\rightarrow$ `false`; Blocked $\rightarrow$ `false` | 48 | 10 | **VERIFIED** | N/A | Xác nhận hàm `canMove` kiểm soát chính xác từng bước di chuyển. |

---

## 5. Kết luận và Đóng góp của Module P2

1. **Tính hoàn chỉnh và độc lập:** Module Dijkstra và Hacker Simulation được chuẩn hóa toàn diện trong `include/Dijkstra.h` và `src/Dijkstra.cpp`, không phụ thuộc biến toàn cục, không xung đột ODR, tương thích 100% với các phân hệ P1 (Graph) và P3 (Min-Cut).
2. **Đáp ứng đầy đủ yêu cầu bài toán:**
   - Tìm kiếm chính xác lộ trình tấn công chi phí tối thiểu.
   - Truy vết tường minh cả danh sách Node lẫn Edge ID.
   - Quản lý ngân sách Token rõ ràng với 3 trạng thái chuẩn (`SUCCESS`, `OVER_BUDGET`, `NO_PATH`).
   - Cung cấp hàm điều kiện di chuyển `canMove()` phục vụ kiểm soát bước đi.
3. **Bộ kiểm thử toàn diện:** Đã xây dựng `tests/hacker_tests.cpp` bao phủ 100% trường hợp cơ bản, trường hợp biên và 10 kịch bản thực nghiệm trên dataset thật, được tích hợp vào CTest của dự án.
