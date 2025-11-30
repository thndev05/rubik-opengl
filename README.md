# Rubik's Cube 3x3x3 Simulator

Chương trình mô phỏng Rubik's Cube 3x3x3 với OpenGL/GLUT, được tách thành nhiều module để dễ đọc và bảo trì.

## Cấu Trúc Thư Mục

```
rubik-opengl/
├── src/                     # Source files
│   ├── main.cpp            # Entry point chính
│   ├── rubik_state.cpp     # Quản lý trạng thái
│   ├── rubik_rotation.cpp  # Logic xoay
│   ├── rubik_animation.cpp # Animation
│   ├── rubik_timer.cpp     # Timer
│   ├── rubik_input.cpp     # Xử lý input
│   └── rubik_render.cpp    # Rendering
├── include/                 # Header files
│   ├── rubik_types.h       # Cấu trúc dữ liệu
│   ├── rubik_constants.h   # Hằng số
│   ├── rubik_state.h       # Quản lý trạng thái
│   ├── rubik_rotation.h    # Logic xoay
│   ├── rubik_animation.h   # Animation
│   ├── rubik_timer.h       # Timer
│   ├── rubik_input.h       # Input
│   └── rubik_render.h      # Render
├── build/                   # Output directory
│   └── rubik.exe           # Executable file
├── build.bat               # Build script
└── README.md               # Documentation
```

## Module Chi Tiết

### Header Files (include/)
- **rubik_types.h** - Định nghĩa các cấu trúc dữ liệu (CubePiece, RubikCube, Animation, Timer, etc.)
- **rubik_constants.h** - Các hằng số (màu sắc, kích thước, tốc độ, etc.)
- **rubik_state.h** - Quản lý trạng thái cube (khởi tạo, reset, shuffle, kiểm tra solved)
- **rubik_rotation.h** - Logic xoay mặt và biến đổi tọa độ
- **rubik_animation.h** - Xử lý animation và queue di chuyển
- **rubik_timer.h** - Timer cho speedsolving (đếm thời gian, moves, TPS)
- **rubik_input.h** - Xử lý input từ bàn phím và chuột
- **rubik_render.h** - Render và hiển thị OpenGL

### Source Files (src/)
- **main.cpp** - Entry point chính (đơn giản, chỉ khởi tạo và gọi các module)
- **rubik_state.cpp** - Implement quản lý trạng thái
- **rubik_rotation.cpp** - Implement logic xoay
- **rubik_animation.cpp** - Implement animation
- **rubik_timer.cpp** - Implement timer
- **rubik_input.cpp** - Implement xử lý input
- **rubik_render.cpp** - Implement rendering

## Compile và Run

### Windows (MinGW)

**Cách 1: Sử dụng build.bat (Khuyến nghị)**
```cmd
build.bat
```

**Cách 2: Compile thủ công**
```powershell
# Compile
g++ -std=c++98 -Wall -Wextra -O2 src\main.cpp src\rubik_state.cpp src\rubik_rotation.cpp src\rubik_animation.cpp src\rubik_timer.cpp src\rubik_input.cpp src\rubik_render.cpp -Iinclude -I"C:\mingw64\include" -L"C:\mingw64\lib" -lfreeglut -lopengl32 -lglu32 -o build\rubik.exe

# Run
.\build\rubik.exe
```

### Linux
```bash
# Tạo thư mục build
mkdir -p build

# Compile
g++ -std=c++98 -Wall -Wextra -O2 src/main.cpp src/rubik_state.cpp src/rubik_rotation.cpp src/rubik_animation.cpp src/rubik_timer.cpp src/rubik_input.cpp src/rubik_render.cpp -Iinclude -lglut -lGLU -lGL -lm -o build/rubik

# Run
./build/rubik
```

## Điều Khiển

### Camera
- **Kéo chuột trái**: Xoay camera
- **Phím mũi tên**: Xoay camera theo từng bước

### Xoay Mặt
- **F/U/R/L/D/B**: Xoay mặt Front/Up/Right/Left/Down/Back theo chiều kim đồng hồ
- **Shift + F/U/R/L/D/B**: Xoay ngược chiều kim đồng hồ

### Chức Năng Khác
- **S**: Trộn cube (20 bước ngẫu nhiên)
- **Space**: Reset về trạng thái đã giải

## Tính Năng

1. **3x3x3 Rubik's Cube đầy đủ** - 27 mảnh với màu sắc chuẩn
2. **Animation mượt mà** - Sử dụng easing function (cubic) 
3. **Move queue** - Xử lý hàng đợi các di chuyển
4. **Speedsolve timer** - Đếm thời gian, số bước, TPS (Turns Per Second)
5. **Auto-scramble** - Trộn tự động
6. **Debug logging** - Ghi log vào file rubik_debug.log

## Module Organization

```
rubik-opengl/
├── src/
│   └── main.cpp (entry point)
│       ├── uses: rubik_state
│       ├── uses: rubik_rotation
│       ├── uses: rubik_animation
│       ├── uses: rubik_timer
│       ├── uses: rubik_input
│       └── uses: rubik_render
├── include/
│   ├── rubik_types.h (data structures)
│   └── rubik_constants.h (constants)
└── build/
    └── rubik.exe (output)
```

## Lợi Ích Của Cấu Trúc Module

1. **Dễ đọc hơn** - Mỗi file tập trung vào một chức năng cụ thể
2. **Dễ bảo trì** - Thay đổi một module không ảnh hưởng các module khác
3. **Dễ mở rộng** - Có thể thêm tính năng mới bằng cách tạo module mới
4. **Compile nhanh hơn** - Chỉ compile lại file thay đổi (khi dùng makefile)
5. **Dễ test** - Có thể test từng module độc lập

## Notes

- Code sử dụng C++98 standard để tương thích tốt nhất
- Tất cả các biến toàn cục được khai báo với `extern` trong header
- Mỗi module có trách nhiệm rõ ràng, không chồng chéo
- Debug log được ghi vào file `rubik_debug.log` để theo dõi

---

**Computer Graphics Final Project**  
Rubik's Cube 3x3x3 Simulator with OpenGL/GLUT
