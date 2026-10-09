# Hợp đồng dữ liệu và validation

## JSON

Root là object chứa `nodes`, `edges` và có thể có `metadata`. Hai trường đầu là array. `metadata` chỉ dành cho ghi chú, không tham gia thuật toán. Loader không suy đoán schema cũ, không tự chuyển CSV hoặc đổi ID. Đây là schema đề xuất có triển khai và test; **chưa đối chiếu với JSON thật vì file chưa được gửi**.

Ví dụ định dạng tối thiểu dùng riêng để minh họa:

```json
{
  "nodes": [
    {"id": 10, "name": "S", "type": "ENTRY", "assets": 0},
    {"id": 90, "name": "T", "type": "TARGET", "assets": 0}
  ],
  "edges": [
    {"id": 7, "from": 10, "to": 90, "weight": 0,
     "relation": "AccessTo", "blocked": false, "capacity": null}
  ]
}
```

| Trường | Quy tắc |
|---|---|
| Node `id` | Số nguyên 0..INT_MAX, duy nhất; không cần liên tiếp |
| `name` | Chuỗi không rỗng/toàn khoảng trắng |
| `type` | `ENTRY`, `ENDPOINT`, `IDENTITY`, `CRITICAL_SYSTEM`, `TARGET` |
| `assets` | Số nguyên 0..INT_MAX; không cộng vào cost đường đi |
| Edge `id` | Số nguyên 0..INT_MAX, duy nhất, bắt buộc trong JSON |
| `from`, `to` | Phải tham chiếu node tồn tại; cạnh có hướng |
| `weight` | Số nguyên 0..INT_MAX; 0 là cạnh tồn tại miễn phí, không phải “không có cạnh” |
| `relation` | `HasSession`, `AdminTo`, `MemberOf`, `AccessTo`, theo `00_START` dòng 6 |
| `blocked` | Boolean; nếu thiếu mặc định false |
| `capacity` | Tùy chọn; null/thiếu nghĩa chưa có giá trị; số nguyên 0..INT_MAX nếu có |

Node/edge bắt buộc đủ các trường không ghi tùy chọn. Từ chối trường lạ ở root/node/edge để bắt lỗi gõ tên, duplicate key, số thực ở trường nguyên, ID trùng, relation/type lạ, tham chiếu node không tồn tại, cost/capacity âm. JSON chuẩn không chấp nhận `//` hoặc `/* */`.

Loader xây graph tạm và chỉ trả về khi thành công; lỗi có vị trí `nodes[i]`/`edges[i]` nếu xảy ra khi đọc bản ghi. `Graph` kiểm tra cùng các ràng buộc khi thêm trực tiếp từ C++. Cạnh song song, chu trình và self-loop được hỗ trợ; chúng không tự bị xóa. Không dùng số lượng 37–38/~55 làm điều kiện hợp lệ.

## Weight và capacity

- `weight`: số token Hacker tiêu tốn khi đi qua cạnh, phù hợp cách `cost[u][v]` được dùng trong source P2. Dijkstra tối thiểu tổng `weight`.
- `capacity`: dành cho bài toán phòng thủ. Nhóm cần thống nhất nó là chi phí vá/ngắt quan hệ hay một đại lượng cụ thể khác trước khi có weighted Min-Cut. Giá trị lớn sẽ khiến cạnh đó đắt hơn để chọn vào cut nếu định nghĩa là chi phí vá.
- Hiện prototype P3 không dùng capacity. Không có max-flow hay mạng dư. Không tự gán `capacity = weight` hoặc 1. Capacity 0 là giá trị có thật, khác với null.
- Tổng cost dùng `std::int64_t`; mỗi weight vẫn `int` như model ban đầu. Budget dùng `std::int64_t` không âm. Cost chỉ có ý nghĩa khi `PathResult::reachable == true`.

## Những kiểm tra chưa thể suy ra từ schema

Tên `HasSession` hay `AdminTo` hợp lệ về cú pháp chưa chứng minh quan hệ có ý nghĩa trong mô hình Active Directory của nhóm. Cần dataset/sơ đồ thực để rà chiều, loại node nguồn–đích và ngữ nghĩa quan hệ. `validationWarnings()` hiện chỉ cảnh báo graph rỗng, thiếu Entry/Target, node cô lập, target không reachable từ Entry trên graph active và thiếu capacity. Đây là cảnh báo, không phải tự sửa dữ liệu.

## Phần cây

BFS/DFS đã có và dùng để kiểm tra khả năng duyệt graph. Chúng không biến attack graph thành cây hoặc gán lại quan hệ cha–con. Không bổ sung Bridge/Articulation Point trong bản này: Excel `05_CORE_CUT` xếp đây là P2, trong khi dataset thật và Min-Cut P0 còn thiếu.
