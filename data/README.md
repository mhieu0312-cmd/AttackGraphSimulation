# Dataset thật chưa có trong file nhận được

Cả ba file `AttackGraphSimulation-main(1).zip`, `(2).zip`, `(3).zip` có cùng SHA-256 và không chứa dataset JSON. JSON duy nhất trong cache build gốc là `CMakeFiles/InstallScripts.json`, không phải dữ liệu graph.

Excel `AttackGraph(2).xlsx` là lộ trình, không có bảng node/edge. Các graph hardcode là demo P2 (4 node, 4 cạnh có weight) và demo P3 (10 node, 11 cạnh, không có weight/capacity). Chúng không phải mạng 37–38 node/~55 cạnh trong kế hoạch.

Vì không có dataset chính để bảo toàn, folder này không tạo `graph.json` giả. `tests/fixtures/p2_demo.json` chỉ chuyển graph mẫu có sẵn thành JSON để thử loader và tích hợp. Node ID, hướng cạnh, weight trong fixture khớp source P2. Edge ID 0–3, các trường type/assets/relation là thông tin bổ sung riêng cho fixture, đã ghi trong metadata; không được dùng như thông tin xác nhận của mạng thật.

Khi nhận file thật, kiểm tra schema và ID trước, không đổi trọng số hay số lượng để ép test đạt. Không tự thay dataset mới nhất bằng dữ liệu từ một ZIP cũ hơn.
