#include "rubik_render.h"
#include "rubik_state.h"
#include "rubik_animation.h"
#include "rubik_input.h"
#include "rubik_timer.h"
#include "rubik_constants.h"
#include <GL/glut.h>
#include <cmath>

int g_windowWidth = 800;
int g_windowHeight = 600;

float clampAngle(float angle, float minAngle, float maxAngle) {
    if (angle < minAngle) {
        return minAngle;
    }
    if (angle > maxAngle) {
        return maxAngle;
    }
    return angle;
}

void rotateAroundAxis(const float axis[3], float angle) {
    float length = sqrt(axis[0] * axis[0] + axis[1] * axis[1] + axis[2] * axis[2]);
    if (length > 0.0001f) {
        float nx = axis[0] / length;
        float ny = axis[1] / length;
        float nz = axis[2] / length;
        glRotatef(angle, nx, ny, nz);
    }
}

// Khởi tạo các cài đặt OpenGL
void initOpenGL() {
    // Tắt culling để vẽ cả 2 mặt của mỗi polygon
    glDisable(GL_CULL_FACE);
    
    // Bật depth test để xử lý độ sâu đúng
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Bật multisampling (nếu hỗ trợ) để làm mượt cạnh
#ifdef GL_MULTISAMPLE
    glEnable(GL_MULTISAMPLE);
#endif
    
    // Cài đặt để vẽ đường viền mượt
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLineWidth(1.5f);
    
    // Màu nền xám đậm
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glShadeModel(GL_SMOOTH);
    
    // Tắt ánh sáng (dùng màu trực tiếp)
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
    glDisable(GL_COLOR_MATERIAL);
}

void drawCubePiece(const CubePiece& piece) {
    const float size = g_rubikCube.pieceSize * 0.5f;
    
    glBegin(GL_QUADS);
    
    // Mặt trước (Z+)
    glColor3fv(piece.colors[0]);
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-size, -size, size);
    glVertex3f(size, -size, size);
    glVertex3f(size, size, size);
    glVertex3f(-size, size, size);
    
    // Mặt sau (Z-)
    glColor3fv(piece.colors[1]);
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(size, -size, -size);
    glVertex3f(-size, -size, -size);
    glVertex3f(-size, size, -size);
    glVertex3f(size, size, -size);
    
    // Mặt trái (X-)
    glColor3fv(piece.colors[2]);
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(-size, -size, -size);
    glVertex3f(-size, -size, size);
    glVertex3f(-size, size, size);
    glVertex3f(-size, size, -size);
    
    // Mặt phải (X+)
    glColor3fv(piece.colors[3]);
    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f(size, -size, size);
    glVertex3f(size, -size, -size);
    glVertex3f(size, size, -size);
    glVertex3f(size, size, size);
    
    // Mặt trên (Y+)
    glColor3fv(piece.colors[4]);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-size, size, size);
    glVertex3f(size, size, size);
    glVertex3f(size, size, -size);
    glVertex3f(-size, size, -size);
    
    // Mặt dưới (Y-)
    glColor3fv(piece.colors[5]);
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(-size, -size, -size);
    glVertex3f(size, -size, -size);
    glVertex3f(size, -size, size);
    glVertex3f(-size, -size, size);
    
    glEnd();
}

// Vẽ toàn bộ Rubik's Cube (27 mảnh)
void drawRubikCube() {
    glPushMatrix();
    
    // Duyệt qua tất cả 27 mảnh
    for (int i = 0; i < 27; i++) {
        const CubePiece& piece = g_rubikCube.pieces[i];
        
        // Bỏ qua mảnh ẩn (nếu có)
        if (!piece.isVisible) {
            continue;
        }
        
        // Tính vị trí thế giới từ toạ độ lưới
        float worldX = (float)piece.position[0] * (g_rubikCube.pieceSize + g_rubikCube.gapSize);
        float worldY = (float)piece.position[1] * (g_rubikCube.pieceSize + g_rubikCube.gapSize);
        float worldZ = (float)piece.position[2] * (g_rubikCube.pieceSize + g_rubikCube.gapSize);
        
        glPushMatrix();
        
        // Kiểm tra xem mảnh này có đang trong animation không
        bool pieceAnimating = g_animation.isActive && isPieceInAnimation(i);
        if (pieceAnimating) {
            // Xác định trục xoay dựa trên mặt đang xoay
            float axisX = 0.0f;
            float axisY = 0.0f;
            float axisZ = 0.0f;
            int axisSign = 1;
            
            switch (g_animation.face) {
                case FRONT:  // Mặt trước: xoay quanh Z+
                    axisZ = 1.0f;
                    axisSign = 1;
                    break;
                case BACK:   // Mặt sau: xoay quanh Z-
                    axisZ = 1.0f;
                    axisSign = -1;
                    break;
                case LEFT:   // Mặt trái: xoay quanh X-
                    axisX = 1.0f;
                    axisSign = -1;
                    break;
                case RIGHT:  // Mặt phải: xoay quanh X+
                    axisX = 1.0f;
                    axisSign = 1;
                    break;
                case UP:     // Mặt trên: xoay quanh Y+
                    axisY = 1.0f;
                    axisSign = 1;
                    break;
                case DOWN:   // Mặt dưới: xoay quanh Y-
                    axisY = 1.0f;
                    axisSign = -1;
                    break;
            }
            
            // Tính góc xoay (xuôi/ngược chiều)
            float angle = g_animation.clockwise ? -g_animation.displayAngle : g_animation.displayAngle;
            angle *= static_cast<float>(axisSign);
            
            // Áp dụng xoay trước khi dịch chuyển
            glRotatef(angle, axisX, axisY, axisZ);
        }
        
        // Dịch chuyển mảnh tới vị trí của nó
        glTranslatef(worldX, worldY, worldZ);
        drawCubePiece(piece);
        glPopMatrix();
    }
    
    glPopMatrix();
}

// Hàm callback hiển thị - vẽ tất cả mọi frame
void display() {
    // Xóa buffer màu và depth
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    
    // Ghi log mỗi 30 frame (giảm spam log)
    static int frameCount = 0;
    if (frameCount++ % 30 == 0 && g_logFile != NULL) {
        fprintf(g_logFile, "DISPLAY: frame=%d\n", frameCount);
        fflush(g_logFile);
    }
    
    // Thiết lập camera
    glTranslatef(0.0f, 0.0f, -CAMERA_DISTANCE);  // Lùi camera ra xa
    rotateAroundAxis(horizontalAxis, cameraAngleY);  // Xoay theo trục ngang
    rotateAroundAxis(verticalAxis, cameraAngleX);    // Xoay theo trục dọc
    
    // Vẽ cube và UI
    drawRubikCube();
    displayTimerOverlay();
    
    // Hoán đổi buffer (double buffering)
    glutSwapBuffers();
}

void reshape(int w, int h) {
    g_windowWidth = w;
    g_windowHeight = h;
    
    if (h == 0) {
        h = 1;
    }
    
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    const float fov = 45.0f;
    const float aspect = (float)w / (float)h;
    const float nearPlane = 0.1f;
    const float farPlane = 100.0f;
    
    gluPerspective(fov, aspect, nearPlane, farPlane);
    glMatrixMode(GL_MODELVIEW);
}
