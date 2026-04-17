# Shortest Path Visualizer (GTKmm + C++)

## 1. Giới thiệu

Dự án của chúng em được viết bằng **C++17 + GTKmm 3** để:

- nạp dữ liệu đồ thị có trọng số (có hướng hoặc vô hướng),
- chạy thuật toán tìm đường đi ngắn nhất,
- theo dõi bảng trạng thái đỉnh + log từng bước,
- so sánh hiệu năng các thuật toán,
- trực quan hóa tiến trình chạy thuật toán ngay trong UI,

Project tập trung vào 3 thuật toán:

- **Dijkstra**
- **Bellman-Ford**
- **BMSSP 2025** (thuật toán tìm đường đi ngắn nhất, nhanh nhất, mới nhất vừa được công bố vào năm 2025)

## 2. Tính năng chính

- Welcome page - giới thiệu chung về nhóm và 
- Nạp đồ thị từ:
  - file `.txt`,
  - hoặc nhập trực tiếp danh sách cạnh.
- Hỗ trợ đồ thị có hướng/vô hướng.
- Chạy thuật toán với đỉnh bắt đầu/kết thúc do người dùng chọn.
- Hiển thị:
  - bảng trạng thái đỉnh (`Đã chốt`, `Chờ xử lý`, `Đang xét`, `Chưa tới`),
  - log cấu trúc theo mức thông tin,
  - kết quả cuối (đường đi, chi phí, thời gian).
- Compare page để so sánh thời gian/bộ nhớ 3 thuật toán.
- Trực quan hóa quá trình cập nhật trọng số giúp các bạn có thể dễ hình dung hơn.

## 3. Công nghệ sử dụng

- **C++17**
- **GTKmm 3.0**
- **Cairo/Cairomm** 
- **CMake** hoặc `g++` + `pkg-config`

## 4. Yêu cầu môi trường

Để chương trình có thể chạy môi trường cần có:

- Trình biên dịch C++ hỗ trợ C++17 (`g++`/`clang++`)
- `pkg-config`
- `gtkmm-3.0` (headers + libs)
- (Tùy chọn) `cmake >= 3.10.

## 5. Build và chạy

```bash
./build.sh
./app_gui.exe
```

## 6. Hướng dẫn sử dụng nhanh

1. Mở ứng dụng.
2. Ở sidebar, chọn **Tải Đồ Thị**.
3. Chọn phương thức nạp:
   - từ file,
   - hoặc nhập trực tiếp.
4. Chọn loại đồ thị (có hướng/vô hướng).
5. Nạp thành công -> xem trang tóm tắt.
6. Nhập đỉnh `Từ` và `Đến`.
7. Chạy thuật toán:
   - Dijkstra,
   - Bellman-Ford,
   - BMSSP 2025.
8. Xem:
   - log,
   - bảng trạng thái đỉnh,
   - panel kết quả.
9. Nhấn **Trực Quan Hóa** để xem animation trên canvas.

## 7. Định dạng dữ liệu đồ thị

Project hỗ trợ **2 format file**.

### Format A: Có nhãn đỉnh

```text
V
label_1 label_2 ... label_V
E
u1 v1 w1
u2 v2 w2
...
uE vE wE
```

### Format B: Không có nhãn đỉnh (tự gán 1..V)

```text
V
E
u1 v1 w1
u2 v2 w2
...
uE vE wE
```

Quy ước:

- `u`, `v` là chỉ số đỉnh từ **1..V**
- `w` là trọng số cạnh (cho phép âm)

### Nhập trực tiếp trên UI

Nhập theo từng dòng:

```text
u v w
```

Ví dụ:

```text
1 2 4
1 3 2
2 4 5
3 4 1
```

## 8. Quy tắc thuật toán

- **Dijkstra**: không dùng được khi đồ thị có cạnh âm.
- **BMSSP 2025**: trong implementation hiện tại chỉ hỗ trợ trọng số không âm.
- **Bellman-Ford**: hỗ trợ cạnh âm, có kiểm tra chu trình âm.

Khi bấm chạy thuật toán mà chưa nạp đồ thị, UI sẽ cảnh báo và chuyển về trang nạp đồ thị.

## 9. Cấu trúc thư mục và chức năng file chính

```text
Project_SPP/
├─ src/
│  ├─ main.cpp
│  ├─ MainWindow.cpp
│  ├─ GraphCanvas.cpp
│  ├─ graph.cpp
│  ├─ algorithms.cpp
│  ├─ AdvancedAlgorithms.cpp
│  └─ comparison.cpp
├─ lib/
│  ├─ MainWindow.h
│  ├─ GraphCanvas.h
│  ├─ graph.h
│  ├─ algorithms.h
│  ├─ AdvancedAlgorithms.h
│  ├─ comparison.h
│  └─ Global.h
├─ data/
├─ CMakeLists.txt
└─ build.sh
```

### Vai trò từng file core

- `src/main.cpp`: entry point GTK app.
- `src/MainWindow.cpp` + `lib/MainWindow.h`: UI chính, luồng thao tác người dùng.
- `src/GraphCanvas.cpp` + `lib/GraphCanvas.h`: vẽ đồ thị và animation native C++.
- `src/graph.cpp` + `lib/graph.h`: mô hình đồ thị, đọc/nạp dữ liệu.
- `src/algorithms.cpp` + `lib/algorithms.h`: Dijkstra, Bellman-Ford, trace/log.
- `src/AdvancedAlgorithms.cpp` + `lib/AdvancedAlgorithms.h`: BMSSP 2025.
- `src/comparison.cpp` + `lib/comparison.h`: đo và so sánh hiệu năng.
- `lib/Global.h`: hằng số và enum dùng chung.

## 10. Dữ liệu output trong runtime

- `data/compare_stats.json`: kết quả so sánh hiệu năng.
- `data/trace.json`: dữ liệu trace (nếu được export bởi logic thuật toán).
- `data/*.txt`: bộ dữ liệu mẫu đầu vào.

## 11. Lỗi thường gặp

- `fatal error: gtkmm.h: No such file or directory`
  - Chưa cài `gtkmm-3.0`.
- Link lỗi `pkg-config`/`gtkmm-3.0 not found`
  - Kiểm tra lại biến môi trường và package `pkg-config`.
- Chạy app nhưng không nạp được file
  - Kiểm tra đúng format ở mục 7 và đường dẫn file.

## 12. Định hướng phát triển

- Thêm test tự động cho parser và thuật toán.
- Chuẩn hóa CMake cho đa nền tảng (Windows/Linux/macOS).
- Tối ưu thêm layout/routing cho đồ thị dày và rất lớn.

---

Nếu bạn cần, mình có thể tách thêm một file `docs/USER_GUIDE.md` để hướng dẫn người dùng cuối và giữ `README.md` gọn hơn cho developer.
