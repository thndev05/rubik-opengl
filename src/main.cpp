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

int main(int argc, char** argv) {
    // Khởi tạo file log debug
    initLogFile();
    
    // Khởi tạo GLUT
    glutInit(&argc, argv);
    
    // Thiết lập chế độ hiển thị
    unsigned int displayMode = GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH;
#ifdef GLUT_MULTISAMPLE
    displayMode |= GLUT_MULTISAMPLE;
#endif
    glutInitDisplayMode(displayMode);
    
    // Thiết lập kích thước và vị trí cửa sổ
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);
    
    // Tạo cửa sổ
    glutCreateWindow("Rubik's Cube - Mô phỏng 3x3x3");
    
    // Khởi tạo cài đặt OpenGL
    initOpenGL();
    
    // Khởi tạo Rubik's Cube
    initRubikCube();
    
    // Kiểm tra tính đúng đắn của phép xoay
    testRotationIdentity();
    
    // Khởi tạo các trục xoay
    updateRotationAxes();
    
    // Khởi tạo seed ngẫu nhiên cho chức năng trộn
    srand((unsigned int)time(NULL));
    
    // Đăng ký các hàm callback
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(keyboardSpecial);
    glutIdleFunc(idle);
    glutIgnoreKeyRepeat(1);
    
    g_lastTimeMs = glutGet(GLUT_ELAPSED_TIME);
    
    // Ghi log hoàn tất khởi tạo
    if (g_logFile != NULL) {
        fprintf(g_logFile, "Ứng dụng khởi tạo thành công\n\n");
        fflush(g_logFile);
    }
    
    std::cout << "=== Mô phỏng Rubik's Cube ===" << std::endl;
    std::cout << "Điều khiển:" << std::endl;
    std::cout << "  Kéo chuột: Xoay camera" << std::endl;
    std::cout << "  Phím mũi tên: Xoay camera" << std::endl;
    std::cout << "  F/U/R/L/D/B: Xoay mặt (Shift để xoay ngược chiều)" << std::endl;
    std::cout << "  S: Trộn cube (20 bước ngẫu nhiên)" << std::endl;
    std::cout << "  Space: Reset về trạng thái đã giải" << std::endl;
    std::cout << "==============================\n" << std::endl;
    
    // Vào vòng lặp chính của GLUT
    glutMainLoop();
    
    // Đóng file log trước khi thoát
    closeLogFile();
    
    return 0;
}
