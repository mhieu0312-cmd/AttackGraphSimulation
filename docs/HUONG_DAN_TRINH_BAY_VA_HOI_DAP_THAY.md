# CẨM NANG BẢO VỆ ĐỒ ÁN VÀ HỎI ĐÁP VỚI THẦY GIÁO
## DÀNH CHO NGƯỜI 2 (P2) - PHÂN HỆ RED TEAM / DIJKSTRA / TOKEN BUDGET

---

## 1. Bài phát biểu mở đầu (Khoảng 1 – 2 phút)

> *"Em chào thầy và hội đồng, trong đề tài **Attack Graph Simulation** của nhóm, em là **Người 2** phụ trách vai trò **Red Team (Mô phỏng kẻ tấn công)**.*
>
> *Nhiệm vụ trọng tâm của em bao gồm 3 phần:*
> 1. *Thứ nhất: Xây dựng thuật toán **Dijkstra** trên đồ thị có hướng để tìm ra lộ trình tấn công có **tổng chi phí nhỏ nhất** từ điểm xâm nhập ban đầu (`PC_LeTan`) tới mục tiêu quan trọng (`CustomerDB`, `SeverDC`, `AdminDB`...).*
> 2. *Thứ hai: Hiện thực hóa cơ chế **Token Budget (Ngân sách tấn công)** và điều kiện di chuyển từng bước **`canMove`** để phân loại chính xác 3 trạng thái của Hacker: `SUCCESS` (chiếm được mục tiêu và còn dư Token), `OVER_BUDGET` (tìm ra đường nhưng thiếu tài nguyên), và `NO_PATH` (mất kết nối).*
> 3. *Thứ ba: Phối hợp cùng P1 và P3 để thực hiện **Re-test (Kiểm tra lại sau khắc phục)**: Chứng minh rằng khi Defender chặn 1 cạnh, Hacker sẽ tự động tìm đường vòng thay thế với chi phí cao hơn; và khi Defender áp dụng lát cắt cực tiểu **Minimum S-T Cut**, đường tấn công sẽ bị triệt hạ hoàn toàn.*
>
> *Sau đây em xin trình bày chi tiết về mã nguồn và kết quả thực nghiệm của phân hệ Red Team."*

---

## 2. Giải thích cấu trúc Code P2 trong 2 phút

Khi thầy yêu cầu: *"Em mở code của em lên và giải thích các hàm chính hoạt động như thế nào?"*:

### Bước 1: Mở file `include/Dijkstra.h`
- **`PathResult`**: Chứa kết quả đường đi gồm `reachable` (có đường hay không), `totalCost` (tổng trọng số), `nodes` (chuỗi đỉnh đi qua) và `edgeIds` (danh sách ID các cạnh tương ứng).
- **`AttackResult`**: Đóng gói kết quả mô phỏng gồm `path`, trạng thái `AttackStatus` (`SUCCESS`, `OVER_BUDGET`, `NO_PATH`), ngân sách ban đầu `initialBudget` và `remainingToken`.
- **`canMove(graph, edgeId, remainingToken)`**: Kiểm tra hacker có thể đi qua một cạnh cụ thể hay không (cạnh tồn tại, không bị chặn và đủ token).

### Bước 2: Mở file `src/Dijkstra.cpp`
Em giải thích luồng thực thi gồm 4 khối chính:
1. **Khối tiền xử lý & Ánh xạ đỉnh (Index Mapping):**
   - Dùng `unordered_map<int, size_t> index` ánh xạ Node ID thực tế trong JSON sang chỉ số mảng $0 \dots n-1$. Giúp thuật toán chạy an toàn với ID không liên tục hoặc ID thưa.
2. **Khối khởi tạo (Initialization):**
   - `distance` gán bằng $\infty$, `distance[start] = 0`.
   - `visited` gán `false`.
   - `parentEdge` gán `-1` để lưu ID của cạnh dẫn tới đỉnh.
3. **Khối vòng lặp Dijkstra (Relaxation Loop):**
   - Mỗi bước chọn đỉnh $u$ chưa `visited` có `distance[u]` nhỏ nhất qua hàm `minDistance()`.
   - Duyệt các cạnh đi ra chưa bị chặn (`graph.getOutGoingEdge()`).
   - Kiểm tra chống tràn số nguyên: `distance[u] > inf - edge->getWeight()`.
   - Thư giãn cạnh: Nếu `distance[u] + weight < distance[v]`, cập nhật `distance[v]` và ghi nhận `parentEdge[v] = edge->getID()`.
4. **Khối truy vết (Path Reconstruction):**
   - Lần ngược từ `target` về `start` bằng `parentEdge`, thêm đỉnh và ID cạnh vào vector rồi `reverse` lại để có thứ tự xuôi chiều.
5. **Khối Hacker Simulation:**
   - So sánh `totalCost` với `budget` để kết luận `SUCCESS`, `OVER_BUDGET` hoặc `NO_PATH`, và tính `remainingToken = budget - totalCost`.

---

## 3. Top 7 câu hỏi thầy giáo hay hỏi nhất & Câu trả lời chuẩn

### ❓ Câu 1: Tại sao em lại chọn thuật toán Dijkstra thay vì BFS, DFS hay Floyd-Warshall?
> **Trả lời:**  
> *"Dạ thưa thầy:  
> - **BFS** chỉ tìm được đường đi ngắn nhất khi đồ thị không có trọng số (mọi cạnh có chi phí bằng 1). Đồ thị của nhóm em là đồ thị có trọng số ($weight \ge 0$), mỗi mối quan hệ có độ khó khác nhau (ví dụ `HasSession` tốn ít chi phí hơn `AdminTo`), nên BFS không thể tìm được đường có chi phí rẻ nhất.  
> - **DFS** chỉ tìm kiếm theo chiều sâu, không đảm bảo tính tối ưu chi phí.  
> - **Floyd-Warshall** tính khoảng cách giữa mọi cặp đỉnh với độ phức tạp $O(V^3)$, quá dư thừa vì bài toán của Red Team chỉ cần tìm đường từ một điểm xâm nhập cụ thể (`Source`) tới một mục tiêu xác định (`Target`).  
> - Do đó, **Dijkstra** với độ phức tạp $O(V^2 + E)$ là giải thuật chuẩn mực và tối ưu nhất cho bài toán này."*

---

### ❓ Câu 2: Độ phức tạp của thuật toán trong bài của em là bao nhiêu?
> **Trả lời:**  
> *"Dạ thưa thầy:  
> - **Độ phức tạp thời gian (Time Complexity):** Bản cài đặt hiện tại sử dụng mảng đánh dấu `visited` và duyệt tuyến tính tìm đỉnh có khoảng cách nhỏ nhất qua hàm `minDistance()`, có độ phức tạp là $O(V^2 + E)$. Với dataset của đồ thị mạng nội bộ HAH Solutions gồm $V = 18$ đỉnh và $E = 24$ cạnh, thuật toán chạy tức thời (dưới 0.1 ms). Nếu hệ thống mở rộng lên hàng nghìn đỉnh, em có thể dễ dàng chuyển sang dùng `std::priority_queue` (Min-Heap) để đạt độ phức tạp $O((V + E) \log V)$.  
> - **Độ phức tạp không gian (Space Complexity):** Là $O(V)$, do chỉ sử dụng các mảng `distance`, `visited`, `parentEdge` có kích thước bằng số đỉnh $V$."*

---

### ❓ Câu 3: Làm thế nào em xử lý được trường hợp đồ thị có cạnh song song (Parallel Edges)?
> **Trả lời:**  
> *"Dạ thưa thầy, giữa hai thành phần mạng có thể tồn tại nhiều quan hệ cùng lúc (ví dụ vừa có `HasSession`, vừa có `AccessTo` với chi phí khác nhau).  
> Nếu truy vết đường đi bằng mảng đỉnh cha `parent[v] = u` theo kiểu truyền thống, ta sẽ không biết được hacker đã đi qua cạnh cụ thể nào giữa $u$ và $v$.  
> Em đã giải quyết vấn đề này bằng cách lưu **`parentEdge[v] = edge->getID()`**. Khi thư giãn cạnh, thuật toán sẽ tự động chọn cạnh có trọng số nhỏ hơn, và lưu chính xác `edgeId` đó. Nhờ vậy, khi truy vết, kết quả vừa có danh sách Node vừa có danh sách Edge ID chính xác 100%."*

---

### ❓ Câu 4: Sự khác nhau giữa `weight` và `capacity` của cạnh là gì? Em có dùng `capacity` không?
> **Trả lời:**  
> *"Dạ thưa thầy, đề tài của nhóm em áp dụng **Mô hình Đa trọng số (Dual-Weight Model)** để phản ánh đúng thực tế an ninh mạng:  
> - **`weight` ($c_{atk}$):** Là chi phí tấn công của Hacker (thời gian, kỹ thuật, rủi ro bị phát hiện). Trọng số này do phân hệ P2 của em sử dụng trong thuật toán Dijkstra để tìm đường rẻ nhất.  
> - **`capacity` ($c_{def}$):** Là chi phí phòng thủ / khắc phục của doanh nghiệp (mức độ gián đoạn hoạt động kinh doanh khi thu hồi quyền). Trọng số này do phân hệ P3 của bạn Defender sử dụng trong bài toán Min-Cut.  
> - Em (P2) **hoàn toàn không dùng `capacity`**, vì hacker chỉ quan tâm đến việc mình tốn bao nhiêu chi phí để chiếm mục tiêu, không quan tâm việc công ty khắc phục tốn bao nhiêu tiền."*

---

### ❓ Câu 5: Hàm `canMove()` hoạt động như thế nào và có ý nghĩa gì?
> **Trả lời:**  
> *"Dạ thưa thầy, hàm `canMove(graph, edgeId, remainingToken)` kiểm tra điều kiện hacker có thể vượt qua một cạnh cụ thể:  
> 1. Cạnh phải tồn tại trong đồ thị.  
> 2. Cạnh chưa bị Defender vô hiệu hóa (`!edge->isBlocked()`).  
> 3. Số Token còn lại phải không âm và đủ chi trả chi phí cạnh (`remainingToken >= edge->getWeight()`).  
> Hàm này phục vụ cho việc kiểm soát bước đi của Hacker và chứng minh tính hợp lệ khi di chuyển qua từng chặng."*

---

### ❓ Câu 6: Điều gì xảy ra khi Defender vá (chặn) một cạnh trên đường tấn công của Hacker?
> **Trả lời:**  
> *"Dạ thưa thầy, em đã thực nghiệm kịch bản này trên dataset thật:  
> - Ban đầu từ `PC_LeTan (36)` tới `CustomerDB (3)`, đường ngắn nhất đi qua cạnh 48 (`PC_Dev1`) với tổng chi phí là **28 Token**.  
> - Khi Defender vá cạnh 48, em cho Hacker chạy lại Dijkstra. Thuật toán tự động tìm ra **đường vòng thay thế** đi qua phòng Kế toán (`PC_KeToan1 -> User_KeToanTruong -> Server_DB -> CustomerDB`) với tổng chi phí tăng lên **29 Token**.  
> - Nếu ban đầu cấp ngân sách đúng 28 Token: Trước khi vá, Hacker `SUCCESS`; sau khi vá, Hacker rơi vào `OVER_BUDGET` do thiếu 1 Token.  
> - Chỉ khi Defender áp dụng lát cắt **Minimum S-T Cut** (chặn toàn bộ `{47, 48, 49}`), Hacker mới hoàn toàn rơi vào trạng thái **`NO_PATH`**."*

---

### ❓ Câu 7: Code của em xử lý các trường hợp biên (Edge cases) như thế nào?
> **Trả lời:**  
> *"Dạ thưa thầy, code của em xử lý chặt chẽ mọi trường hợp biên:  
> 1. **Source trùng Target ($S = T$):** Chi phí bằng 0, không tốn Token, `SUCCESS`.  
> 2. **Đồ thị mất kết nối hoặc đi ngược chiều:** Trả về `NO_PATH`.  
> 3. **Ngân sách âm ($Budget < 0$):** Ném ngoại lệ `invalid_argument`.  
> 4. **Trọng số cạnh bằng 0:** Dijkstra xử lý bình thường, không gây lặp vô hạn.  
> 5. **Tổng chi phí rất lớn:** Có điều kiện kiểm tra chống tràn số nguyên 64-bit `int64_t`."*

---

## 4. Kịch bản Demo thực chiến từng bước (Dành cho buổi bảo vệ)

Khi thầy yêu cầu demo, em mở PowerShell và gõ lần lượt 4 lệnh sau:

### Lệnh 1: Chạy toàn bộ Test Suite của P2 (Chứng minh code đúng 100%)
```powershell
.\AttackGraphSimulation-main\hacker_tests.exe all
```
*Kết quả in ra:*
```text
dijkstra_basic PASS
can_move PASS
budget_states PASS
edge_cases PASS
dataset_10_scenarios PASS

>>> ALL P2 HACKER TESTS PASSED SUCCESSFULLY! <<<
```
👉 *Lời thoại:* "Dạ thưa thầy, toàn bộ unit test và 10 kịch bản thực nghiệm của phân hệ Red Team đều vượt qua 100%."

---

### Lệnh 2: Mô phỏng mặc định (SUCCESS)
```powershell
.\AttackGraphSimulation-main\AttackGraph.exe "AttackGraphSimulation-main/data/graph.json" 36 3 30
```
👉 *Lời thoại:* "Với Budget = 30, đường đi tối ưu từ PC_LeTan tới CustomerDB có Cost = 28, Hacker thành công với 2 Token còn lại. Sau đó Defender chạy Min-Cut chặn 3 cạnh, Hacker bị chặn hoàn toàn (NO_PATH)."

---

### Lệnh 3: Mô phỏng thiếu Token (OVER_BUDGET)
```powershell
.\AttackGraphSimulation-main\AttackGraph.exe "AttackGraphSimulation-main/data/graph.json" 36 3 27
```
👉 *Lời thoại:* "Khi hạ Budget xuống 27, dù vẫn có đường đi tới đích nhưng do thiếu 1 Token (Cost 28 > 27), hệ thống xác định trạng thái là OVER_BUDGET."

---

### Lệnh 4: Mô phỏng Defender vá 1 cạnh làm Hacker phải đi đường vòng
```powershell
.\AttackGraphSimulation-main\AttackGraph.exe "AttackGraphSimulation-main/data/graph.json" 36 3 28 --patch 48
```
👉 *Lời thoại:* "Thưa thầy, đây là kịch bản rất thú vị: Với ngân sách 28 Token, trước khi vá Hacker đạt SUCCESS (Cost 28 = Budget 28). Nhưng khi ta vá cạnh 48, Hacker buộc phải đi đường vòng qua phòng Kế toán với Cost = 29 > 28, dẫn tới Hacker rơi vào trạng thái OVER_BUDGET. Điều này chứng minh việc vô hiệu hóa permission đã làm thay đổi hành vi tấn công."
