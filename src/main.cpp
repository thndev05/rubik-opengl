/*
 * Rubik's Cube - Mô phỏng Rubik 3x3x3
 * Đồ án cuối kỳ Đồ họa Máy tính
 * 
 * Đây là phiên bản module hóa của chương trình mô phỏng Rubik's Cube với:
 * - Rubik 3x3x3 đầy đủ với 27 mảnh
 * - Animation mượt mà với easing
 * - Điều khiển camera (kéo chuột & phím mũi tên)
 * - Điều khiển xoay mặt (phím F/U/R/L/D/B)
 * - Tính năng timer và speedsolving
 * - Chức năng trộn tự động
 * 
 * Biên dịch (Windows/MinGW - PowerShell):
 * g++ -std=c++98 -Wall -Wextra -O2 src\main.cpp src\rubik_state.cpp src\rubik_rotation.cpp src\rubik_animation.cpp src\rubik_timer.cpp src\rubik_input.cpp src\rubik_render.cpp -Iinclude -I"C:\mingw64\include" -L"C:\mingw64\lib" -lfreeglut -lopengl32 -lglu32 -o build\rubik.exe
 * 
 * Hoặc dùng build.bat:
 * build.bat
 * 
 * Biên dịch (Linux):
 * g++ -std=c++98 -Wall -Wextra -O2 src/main.cpp src/rubik_state.cpp src/rubik_rotation.cpp src/rubik_animation.cpp src/rubik_timer.cpp src/rubik_input.cpp src/rubik_render.cpp -Iinclude -lglut -lGLU -lGL -lm -o build/rubik
 * 
 * Điều khiển:
 * - Kéo chuột: Xoay góc nhìn camera
 * - Phím mũi tên: Xoay góc nhìn camera
 * - F/U/R/L/D/B: Xoay mặt Front/Up/Right/Left/Down/Back theo chiều kim đồng hồ
 * - Shift + F/U/R/L/D/B: Xoay mặt ngược chiều kim đồng hồ
 * - S: Trộn cube (20 bước ngẫu nhiên)
 * - Space: Reset cube về trạng thái đã giải
 */

#include <GL/glut.h>
#include <cstdlib>
#include <ctime>
#include <iostream>

// Include tất cả các module headers
#include "rubik_types.h"
#include "rubik_constants.h"
#include "rubik_state.h"
#include "rubik_rotation.h"
#include "rubik_animation.h"
#include "rubik_timer.h"
#include "rubik_input.h"
#include "rubik_render.h"

/**
 * Hàm chính (entry point) của chương trình.
 * Nơi khởi tạo cửa sổ, thiết lập OpenGL và bắt đầu vòng lặp sự kiện.
 */
int main(int argc, char** argv) {
    // 1. Khởi tạo hệ thống ghi nhật ký (logging) để debug lỗi
    initLogFile(); // Mở file log và ghi thời gian bắt đầu
    
    // 2. Khởi tạo thư viện GLUT (OpenGL Utility Toolkit)
    // GLUT giúp quản lý cửa sổ và sự kiện đầu vào một cách dễ dàng
    glutInit(&argc, argv); // Truyền tham số dòng lệnh cho GLUT xử lý
    
    // 3. Thiết lập chế độ hiển thị (Display Mode)
    // GLUT_DOUBLE: Sử dụng Double Buffering (2 bộ đệm) để tránh hiện tượng nhấp nháy khi vẽ
    // GLUT_RGB: Sử dụng hệ màu Red-Green-Blue
    // GLUT_DEPTH: Sử dụng Depth Buffer (Z-buffer) để xử lý chiều sâu (vật trước che vật sau)
    unsigned int displayMode = GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH;
    
    // Nếu hệ thống hỗ trợ khử răng cưa (Multisample Anti-Aliasing), bật nó lên
#ifdef GLUT_MULTISAMPLE
    displayMode |= GLUT_MULTISAMPLE; // Thêm cờ Multisample vào chế độ hiển thị
#endif
    glutInitDisplayMode(displayMode); // Áp dụng chế độ hiển thị đã cấu hình
    
    // 4. Thiết lập cửa sổ ứng dụng
    glutInitWindowSize(800, 600); // Đặt kích thước cửa sổ là 800x600 pixel
    glutInitWindowPosition(100, 100); // Đặt vị trí cửa sổ tại toạ độ (100, 100) trên màn hình
    
    // Tạo cửa sổ với tiêu đề đã định
    // Hàm này trả về ID của cửa sổ (nhưng ta không cần lưu lại ở đây)
    glutCreateWindow("Rubik's Cube - Mô phỏng 3x3x3");
    
    // 5. Khởi tạo các cài đặt OpenGL cụ thể cho ứng dụng
    // Bao gồm: màu nền, ánh sáng, vật liệu, phép chiếu (projection), v.v.
    initOpenGL(); // Hàm này nằm trong rubik_render.cpp
    
    // 6. Khởi tạo trạng thái logic của khối Rubik
    // Tạo 27 mảnh, gán màu sắc ban đầu cho các mặt
    initRubikCube(); // Hàm này nằm trong rubik_state.cpp
    
    // 7. Kiểm tra tính toàn vẹn của logic xoay (Unit Test nhỏ)
    // Thử xoay 4 lần một mặt xem có về trạng thái cũ không
    testRotationIdentity(); // Hàm này nằm trong rubik_state.cpp
    
    // 8. Cập nhật các vector trục xoay cho animation
    // Đảm bảo các trục X, Y, Z được định nghĩa đúng để dùng cho glRotatef
    updateRotationAxes(); // Hàm này nằm trong rubik_animation.cpp
    
    // 9. Khởi tạo bộ sinh số ngẫu nhiên (Random Seed)
    // Dùng thời gian hiện tại làm seed để mỗi lần chạy có chuỗi trộn (shuffle) khác nhau
    srand((unsigned int)time(NULL));
    
    // 10. Đăng ký các hàm Callback cho GLUT
    // Callback là các hàm sẽ được GLUT gọi tự động khi có sự kiện tương ứng
    
    glutDisplayFunc(display);       // Gọi khi cần vẽ lại màn hình (trong rubik_render.cpp)
    glutReshapeFunc(reshape);       // Gọi khi cửa sổ thay đổi kích thước (trong rubik_render.cpp)
    glutMouseFunc(mouse);           // Gọi khi có sự kiện click chuột (trong rubik_input.cpp)
    glutMotionFunc(motion);         // Gọi khi di chuyển chuột (trong rubik_input.cpp)
    glutKeyboardFunc(keyboard);     // Gọi khi nhấn phím ký tự thường (trong rubik_input.cpp)
    glutKeyboardUpFunc(keyboardUp); // Gọi khi nhả phím (trong rubik_input.cpp)
    glutSpecialFunc(keyboardSpecial); // Gọi khi nhấn phím đặc biệt (mũi tên, F1-F12...)
    glutIdleFunc(idle);             // Gọi liên tục khi chương trình rảnh (dùng cho animation)
    
    // Ngăn chặn việc lặp lại phím khi giữ (chỉ nhận sự kiện nhấn xuống một lần)
    // Giúp việc xoay Rubik không bị quá nhanh hoặc mất kiểm soát
    glutIgnoreKeyRepeat(1);
    
    // Lấy thời gian hiện tại của hệ thống (tính bằng mili giây)
    // Dùng để tính delta time cho animation mượt mà
    g_lastTimeMs = glutGet(GLUT_ELAPSED_TIME);
    
    // Ghi log xác nhận khởi tạo thành công
    if (g_logFile != NULL) {
        fprintf(g_logFile, "Ứng dụng khởi tạo thành công\n\n");
        fflush(g_logFile); // Đẩy dữ liệu từ bộ đệm xuống file ngay lập tức
    }
    
    // In hướng dẫn sử dụng ra màn hình Console
    std::cout << "=== Mô phỏng Rubik's Cube ===" << std::endl;
    std::cout << "Điều khiển:" << std::endl;
    std::cout << "  Kéo chuột: Xoay camera" << std::endl;
    std::cout << "  Phím mũi tên: Xoay camera" << std::endl;
    std::cout << "  F/U/R/L/D/B: Xoay mặt (Shift để xoay ngược chiều)" << std::endl;
    std::cout << "  S: Trộn cube (20 bước ngẫu nhiên)" << std::endl;
    std::cout << "  Space: Reset về trạng thái đã giải" << std::endl;
    std::cout << "==============================\n" << std::endl;
    
    // 11. Bắt đầu vòng lặp chính của GLUT (Event Loop)
    // Hàm này sẽ không bao giờ trả về (vòng lặp vô hạn)
    // Nó sẽ liên tục xử lý sự kiện và gọi các hàm callback đã đăng ký
    glutMainLoop();
    
    // Code dưới đây thực tế sẽ không bao giờ chạy được do glutMainLoop() chặn lại
    // Tuy nhiên, để đảm bảo tính đúng đắn về mặt cấu trúc, ta vẫn để lệnh đóng file
    closeLogFile();
    
    return 0; // Trả về 0 báo hiệu chương trình kết thúc thành công
}
