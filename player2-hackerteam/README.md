# BÀN GIAO NGƯỜI 2 — RED TEAM / HACKER

Gói này là phần hoàn chỉnh để ghép vào Attack Graph Simulation. Bản code dùng mảng tĩnh và Dijkstra `O(V^2)`, phù hợp graph khoảng 38 node và sát cách trình bày trên slide môn học.

## Sản phẩm có trong gói

| Thành phần | File | Người dùng |
|---|---|---|
| Mã C++ Dijkstra, Budget, path reconstruction, Hacker simulation | `code/p2_hacker_dijkstra.cpp` | Người 1 ghép vào chương trình chính |
| Hợp đồng ghép code | `tai_lieu/01_huong_dan_ghep_code.md` | Người 1 và Người 3 |
| Bảng test + expected result | `tai_lieu/02_test_cases.md` | Cả nhóm |
| Đoạn report, bố cục slide, lời demo | `tai_lieu/03_report_ppt_demo.md` | Cả nhóm |

## Cách chạy bản demo riêng

```text
g++ -std=c++17 code/p2_hacker_dijkstra.cpp -o p2_demo
./p2_demo
```

## Quy tắc chính

- `cost[u][v]` là token cần cho bước tấn công `u -> v`.
- `cost[u][v] = INF` nghĩa là không có cạnh `u -> v`.
- `blocked[u][v] = true` nghĩa là Defender đã patch cạnh đó.
- Hacker thành công khi đường rẻ nhất tồn tại và `minCost <= budget`.

Không xóa cạnh sau patch. Chỉ đổi `blocked[u][v]` rồi gọi lại `hackerSimulation()`.
