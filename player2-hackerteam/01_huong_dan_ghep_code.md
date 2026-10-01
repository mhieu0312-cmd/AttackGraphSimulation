# Hợp đồng ghép code giữa ba người

## Người 1 cần làm

1. Gọi `initializeGraph(n)` một lần khi tạo graph.
2. Với mỗi cạnh tấn công `u -> v` trong dataset, gán `cost[u][v] = tokenCost`.
3. Gọi `hackerSimulation(n, source, target, budget)` trước patch.
4. Sau khi có cạnh cần chặn, gán `blocked[u][v] = true`.
5. Gọi lại `hackerSimulation(n, source, target, budget)` để re-test.

## Người 3 cần gửi cho Người 1

Danh sách cạnh Min-Cut dưới dạng cặp `(u, v)`, ví dụ `(2, 3)`. Người 1 dùng cặp đó để đặt `blocked[2][3] = true`.

## Người 2 cung cấp

- `dijkstra()`: tính chi phí ngắn nhất.
- `printPath()`: in path từ source đến target.
- `hackerSimulation()`: in Before/After, cost, budget, trạng thái SUCCESS/FAIL.

## Lưu ý quan trọng

Code hiện giả sử một cặp node chỉ có tối đa một cạnh có hướng. Nếu dataset có hai cạnh cùng `u -> v`, nhóm giữ cạnh có token cost nhỏ hơn cho MVP, hoặc thống nhất mở rộng cấu trúc cạnh sau.
