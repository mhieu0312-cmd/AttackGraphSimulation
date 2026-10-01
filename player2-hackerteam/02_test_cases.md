# Test cases phần Người 2

| ID | Thiết lập | Budget | Kết quả mong đợi |
|---|---|---:|---|
| T1 | Hai đường có cost 13 và 19 | 15 | Chọn đường cost 13 |
| T2 | Đường rẻ nhất cost 13 | 15 | SUCCESS, còn 2 token |
| T3 | Đường rẻ nhất cost đúng bằng budget | 13 | SUCCESS, còn 0 token |
| T4 | Đường rẻ nhất cost 13 | 12 | FAIL vì vượt budget |
| T7 | Block cạnh thuộc đường cũ | 15 | Không dùng lại đường cũ |
| T10 | Trước patch: path cost 13 | 15 | SUCCESS |
| T11 | Block `2 -> 3`; đường thay thế cost 19 | 15 | FAIL vì vượt budget |
| R1 | Không có đường source -> target | 15 | FAIL: không còn đường |

## Kết quả demo đã chạy

| Lần chạy | Path | Cost | Budget | Status |
|---|---|---:|---:|---|
| Before Patch | 0 -> 1 -> 2 -> 3 | 13 | 15 | SUCCESS |
| After Patch: block 2 -> 3 | 0 -> 1 -> 3 | 19 | 15 | FAIL |
