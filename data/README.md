# Dataset chính thức

`graph.json` là mô hình học thuật có hướng gồm **18 nodes, 24 edges, 5 loại node**. Không phải schema BloodHound hoặc dữ liệu thu thập từ mạng thật.

- Source gợi ý: **18 — PC_LeTan**.
- Targets: **1 — SeverDC**, **2 — AdminDB**, **3 — CustomerDB**, **8 — BackupServer**.
- Giữ nguyên tên, topology, weight, capacity và blocked của dataset hiện tại.
- Weight dùng cho Dijkstra; capacity dùng cho chi phí patch của Min-Cut; assets không tham gia hai thuật toán.

Schema được hỗ trợ: `docs/DATASET_SCHEMA.md`. Có thể thay JSON hợp lệ rồi chạy lại, không sửa code hoặc rebuild. Metadata chỉ là ghi chú; chương trình chọn Source/Target qua input hoặc CLI, không đọc ID gợi ý thành cấu hình bắt buộc.

Ví dụ chạy từ folder project:

```sh
AttackGraph data/graph.json 18 3 30
AttackGraph path/to/another.json
```

Chạy không tham số trong CLion ưu tiên `data/graph.json` tại folder source. Khi kiểm tra dataset khác hoặc bản đóng gói, truyền đường dẫn JSON rõ ràng để tránh chọn nhầm file.
