# Nội dung report, PPT và demo của Người 2

## Đoạn report có thể dùng

Red Team được mô hình hóa bằng Attack Graph có hướng và có trọng số. Mỗi node biểu diễn một thực thể trong mạng, còn cạnh `u -> v` biểu diễn một bước tấn công có thể thực hiện từ `u` sang `v`. Trọng số `cost[u][v]` là số token cần để thực hiện bước đó. Hệ thống sử dụng thuật toán Dijkstra để tìm đường đi có tổng token nhỏ nhất từ source đến target.

Với budget `B`, Hacker thành công khi tồn tại đường đi và chi phí nhỏ nhất không vượt quá `B`. Khi Defender patch một cạnh, chương trình gán `blocked[u][v] = true`. Dijkstra sẽ bỏ qua cạnh này ở lần chạy lại. Do đó, hệ thống so sánh được kết quả trước và sau patch để đánh giá Defender có làm lộ trình tấn công bị chặn hoặc trở nên quá đắt hay không.

## Ba slide của Người 2

1. **Mô hình Red Team:** node, cạnh có hướng, cost và budget 15.
2. **Dijkstra + quy tắc quyết định:** tìm `minCost`; SUCCESS khi `minCost <= budget`.
3. **Kết quả Before/After Patch:** Before có path cost 13 nên SUCCESS; After block cạnh 2 -> 3, path cost 19 nên FAIL.

## Lời demo

“Em chạy Dijkstra từ Entry đến Target để tìm đường có tổng token nhỏ nhất. Trước patch, đường rẻ nhất tốn 13 token, nhỏ hơn budget 15 nên Hacker thành công. Sau khi Defender block cạnh Min-Cut, chương trình chạy lại Dijkstra. Đường cũ không còn được xét; đường thay thế tốn 19 token nên Hacker thất bại vì vượt budget.”
