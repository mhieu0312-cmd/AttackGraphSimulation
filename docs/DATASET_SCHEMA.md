# Schema JSON hiện tại

Schema này mô tả `src/LoadDataset.cpp`, `src/Graph.cpp` và `src/MinCut.cpp` hiện tại. Các báo cáo kiểm thử cũ trong `docs/` là tài liệu lịch sử.

Root phải là object có hai array `nodes`, `edges`. `metadata` là object tùy chọn, dùng để ghi chú và không tham gia thuật toán. Không giới hạn số node/cạnh ở 18/24; đó là quy mô dataset chính thức hiện tại.

```json
{
  "nodes": [
    {"id": 10, "name": "S", "type": "ENTRY", "assets": 0},
    {"id": 90, "name": "T", "type": "TARGET", "assets": 0}
  ],
  "edges": [
    {"id": 7, "from": 10, "to": 90, "weight": 3,
     "capacity": 2, "relation": "AccessTo", "blocked": false}
  ]
}
```

| Trường | Quy tắc |
|---|---|
| Node `id` | Số nguyên 0..INT_MAX, duy nhất trong nodes, không cần liên tiếp |
| `name` | Chuỗi không rỗng hoặc chỉ chứa khoảng trắng |
| `type` | ENTRY, ENDPOINT, IDENTITY, CRITICAL_SYSTEM, TARGET |
| `assets` | Số nguyên 0..INT_MAX, bắt buộc để tương thích model; không tham gia Dijkstra/Min-Cut |
| Edge `id` | Số nguyên 0..INT_MAX, duy nhất trong edges, không cần liên tiếp |
| `from`, `to` | ID của node tồn tại; cạnh có hướng |
| `weight` | Số nguyên 0..INT_MAX, chi phí attacker; 0 vẫn là cạnh đi được |
| `capacity` | Số nguyên 0..INT_MAX, bắt buộc; không chấp nhận thiếu/null |
| `relation` | HasSession, AdminTo, MemberOf, AccessTo |
| `blocked` | Boolean tùy chọn, mặc định false |

Loader từ chối trường lạ ở root/node/edge, key JSON trùng, dữ liệu thừa sau JSON, trường bắt buộc thiếu, kiểu dữ liệu sai, ID trùng, tham chiếu không tồn tại và số ngoài miền cho phép. Trường trong metadata được phép tùy ý, nhưng key vẫn không được trùng. JSON không hỗ trợ comment.

Lỗi đọc bản ghi có vị trí `nodes[i]` hoặc `edges[i]`, chỉ số bắt đầu từ 0. Lỗi cú pháp/key trùng xảy ra trước khi dựng Graph và được báo tại root. Graph được tạo tạm; loader chỉ trả về khi đọc thành công.

Cạnh song song, cạnh ngược chiều, chu trình và self-loop được hỗ trợ. Không tự xóa cạnh hoặc thay đổi quan hệ. Tên relation hợp lệ về schema không chứng minh đó là quyền thực tế trong Active Directory.

## Thuật toán và trạng thái

- Dijkstra tối thiểu tổng weight, dùng int64_t cho cost và budget. Cạnh blocked không được đi qua.
- Edmonds–Karp dùng capacity để tính Max-Flow và Minimum S-T Cut. Assets và weight không tham gia flow.
- Capacity 0 là chi phí patch bằng 0; cạnh vẫn đi được nếu chưa blocked. Cut có thể chứa cạnh capacity 0 để ngắt kết nối thực sự.
- `minimumSTCut()` trả ID cạnh gốc, không thay đổi Graph; source phải khác target. Khi không có đường đi, flow/cut bằng 0 và tập cạnh rỗng.
- `runSimulation()` chạy Attack, Defense, Patch, Re-Attack. Source trùng target hoặc S-T đã mất kết nối thì không patch. Sau patch cấp lại budget ban đầu.
- Patch chỉ cập nhật blocked trong bộ nhớ; không xóa cạnh hoặc ghi lại JSON. `blockEdges()` kiểm tra toàn bộ ID trước khi thay đổi trạng thái.
- Reset bằng cách copy Graph ban đầu hoặc load lại dataset, giữ đúng các cạnh vốn blocked trong JSON.

Graph rỗng, thiếu ENTRY/TARGET, node cô lập hoặc TARGET không reachable từ bất kỳ ENTRY nào là warning. CLI tương tác cần ít nhất một node; chế độ demo cần ENTRY và TARGET. Không tự sửa dữ liệu theo warning.
