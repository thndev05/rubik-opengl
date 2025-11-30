#include "rubik_animation.h"
#include "rubik_state.h"
#include "rubik_rotation.h"
#include "rubik_timer.h"
#include "rubik_constants.h"
#include <GL/glut.h>
#include <cstdio>

// Trạng thái animation hiện tại
// isActive: Đang chạy animation hay không
// face: Mặt đang xoay (FRONT, BACK, LEFT, RIGHT, UP, DOWN)
// clockwise: Hướng xoay (true = xuôi chiều, false = ngược chiều)
// isScrambleMove: Đây có phải là nước đi trộn hay không
// currentAngle: Góc đã xoay hiện tại (0-90 độ)
// targetAngle: Góc mục tiêu (thường là 90 độ)
// speed: Tốc độ xoay (độ/giây)
// displayAngle: Góc hiển thị sau khi áp dụng easing
// affectedIndices: Chỉ số của 9 mảnh đang xoay
RotationAnimation g_animation = {
    false,
    FRONT,
    true,
    false,
    0.0f,
    90.0f,
    ROTATION_SPEED_DEG_PER_SEC,
    0.0f,
    {-1, -1, -1, -1, -1, -1, -1, -1, -1}
};

// Hàng đợi các nước đi chờ thực hiện
MoveQueue g_moveQueue = {{0}, {false}, {false}, 0, 0};

// Thời gian frame trước (đơn vị: milliseconds)
int g_lastTimeMs = 0;

// Trạng thái phím đang được giữ
bool g_keyHeld[256] = {false};

// Số nước trộn còn lại đang chờ
int g_scrambleMovesPending = 0;

// Hàm easing cubic - Tạo hiệu ứng chuyển động mượt mà
// Đầu vào: t trong khoảng [0, 1] (0 = bắt đầu, 1 = kết thúc)
// Đầu ra: Giá trị đã được easing trong [0, 1]
// Animation sẽ chậm ở đầu, nhanh ở giữa, và chậm lại ở cuối
float easeInOutCubic(float t) {
    // Đảm bảo t nằm trong [0, 1]
    if (t < 0.0f) {
        t = 0.0f;
    } else if (t > 1.0f) {
        t = 1.0f;
    }
    
    // Nửa đầu: tăng tốc (ease-in)
    if (t < 0.5f) {
        return 4.0f * t * t * t;
    }
    
    // Nửa sau: giảm tốc (ease-out)
    float f = (2.0f * t) - 2.0f;
    return 0.5f * f * f * f + 1.0f;
}

// Kiểm tra xem một mảnh có đang trong animation hay không
// Tham số:
//   pieceIndex: Chỉ số của mảnh cần kiểm tra (0-26)
// Trả về:
//   true nếu mảnh này đang trong danh sách 9 mảnh bị ảnh hưởng bởi animation hiện tại
bool isPieceInAnimation(int pieceIndex) {
    // Nếu không có animation nào đang chạy
    if (!g_animation.isActive) {
        return false;
    }
    
    // Duyệt qua 9 mảnh bị ảnh hưởng
    for (int i = 0; i < 9; i++) {
        if (g_animation.affectedIndices[i] == pieceIndex) {
            return true;
        }
    }
    return false;
}

// Hủy animation hiện tại và xóa toàn bộ hàng đợi
// Dùng khi reset cube hoặc muốn dừng tất cả chuyển động
void cancelAnimationAndQueue() {
    // Tắt animation
    g_animation.isActive = false;
    g_animation.isScrambleMove = false;
    g_animation.currentAngle = 0.0f;
    g_animation.displayAngle = 0.0f;
    
    // Xóa danh sách 9 mảnh bị ảnh hưởng
    for (int i = 0; i < 9; i++) {
        g_animation.affectedIndices[i] = -1;
    }
    
    // Xóa hàng đợi chờ
    g_moveQueue.count = 0;
    g_moveQueue.head = 0;
    for (int i = 0; i < MOVE_QUEUE_CAPACITY; i++) {
        g_moveQueue.scrambleFlags[i] = false;
    }
}

// Lấy một nước đi từ hàng đợi
// Tham số:
//   face: Biến tham chiếu để lưu mặt cần xoay
//   clockwise: Biến tham chiếu để lưu hướng xoay
//   isScrambleMove: Biến tham chiếu để lưu cờ scramble hay không
// Trả về:
//   true nếu lấy thành công, false nếu hàng đợi rỗng
bool dequeueQueuedMove(Face& face, bool& clockwise, bool& isScrambleMove) {
    // Hàng đợi rỗng
    if (g_moveQueue.count == 0) {
        return false;
    }
    
    // Lấy nước đi từ đầu hàng đợi
    int idx = g_moveQueue.head;
    face = static_cast<Face>(g_moveQueue.moves[idx]);
    clockwise = g_moveQueue.dirs[idx];
    isScrambleMove = g_moveQueue.scrambleFlags[idx];
    
    // Xóa cờ scramble
    g_moveQueue.scrambleFlags[idx] = false;
    
    // Di chuyển head và giảm count (hàng đợi vòng tròn)
    g_moveQueue.head = (g_moveQueue.head + 1) % MOVE_QUEUE_CAPACITY;
    g_moveQueue.count--;
    
    // Reset head nếu hàng đợi rỗng
    if (g_moveQueue.count == 0) {
        g_moveQueue.head = 0;
    }
    return true;
}

// Bắt đầu một animation xoay mặt
// Nếu đang có animation khác chạy, sẽ thêm vào hàng đợi
// Tham số:
//   face: Mặt cần xoay (FRONT, BACK, LEFT, RIGHT, UP, DOWN)
//   clockwise: true = xuôi chiều kim đồng hồ, false = ngược chiều
//   isScrambleMove: Đánh dấu đây là nước đi trộn (không đếm vào timer)
void startRotation(Face face, bool clockwise, bool isScrambleMove) {
    // Kiểm tra tính hợp lệ của face
    if (face < FRONT || face > DOWN) {
        return;
    }
    
    // Nếu đang có animation chạy, thêm vào hàng đợi
    if (g_animation.isActive) {
        // Hàng đợi đầy, bỏ qua nước đi này
        if (g_moveQueue.count >= MOVE_QUEUE_CAPACITY) {
            if (g_logFile != NULL) {
                const char* faceNames[] = {"FRONT", "BACK", "LEFT", "RIGHT", "UP", "DOWN"};
                double tsMs = getLogTimestampMs();
                fprintf(g_logFile, "[%010.3f ms] QUEUE FULL: drop %s %s\n",
                        tsMs,
                        faceNames[face],
                        clockwise ? "CW" : "CCW");
                fflush(g_logFile);
            }
        } else {
            // Thêm vào cuối hàng đợi
            int idx = (g_moveQueue.head + g_moveQueue.count) % MOVE_QUEUE_CAPACITY;
            g_moveQueue.moves[idx] = static_cast<int>(face);
            g_moveQueue.dirs[idx] = clockwise;
            g_moveQueue.scrambleFlags[idx] = isScrambleMove;
            g_moveQueue.count++;
            if (g_logFile != NULL) {
                const char* faceNames[] = {"FRONT", "BACK", "LEFT", "RIGHT", "UP", "DOWN"};
                double tsMs = getLogTimestampMs();
                fprintf(g_logFile, "[%010.3f ms] ANIM QUEUED %s %s | queue=%d\n",
                        tsMs,
                        faceNames[face],
                        clockwise ? "CW" : "CCW",
                        g_moveQueue.count);
                fflush(g_logFile);
            }
        }
        return;
    }
    
    // Bắt đầu animation mới
    onMoveStarted();  // Thông báo cho timer (nếu đang chạy)
    
    g_animation.isActive = true;
    g_animation.face = face;
    g_animation.clockwise = clockwise;
    g_animation.isScrambleMove = isScrambleMove;
    g_animation.currentAngle = 0.0f;      // Bắt đầu từ 0 độ
    g_animation.displayAngle = 0.0f;
    g_animation.targetAngle = 90.0f;      // Mục tiêu 90 độ
    g_animation.speed = ROTATION_SPEED_DEG_PER_SEC;
    
    // Lấy danh sách 9 mảnh thuộc mặt này
    getFaceIndices(face, g_animation.affectedIndices);
    if (g_logFile != NULL) {
        const char* faceNames[] = {"FRONT", "BACK", "LEFT", "RIGHT", "UP", "DOWN"};
        double tsMs = getLogTimestampMs();
        fprintf(g_logFile, "[%010.3f ms] ANIM START %s %s | queue=%d\n",
                tsMs,
                faceNames[face],
                clockwise ? "CW" : "CCW",
                g_moveQueue.count);
        fflush(g_logFile);
    }
    glutPostRedisplay();
}

// Cập nhật animation mỗi frame
// Tham số:
//   deltaTime: Thời gian trôi qua kể từ frame trước (đơn vị: giây)
void updateAnimation(float deltaTime) {
    // Không có animation nào đang chạy
    if (!g_animation.isActive) {
        return;
    }
    
    // Tăng góc xoay dựa trên tốc độ và deltaTime
    g_animation.currentAngle += g_animation.speed * deltaTime;
    
    // Giới hạn không vượt quá góc mục tiêu
    if (g_animation.currentAngle > g_animation.targetAngle) {
        g_animation.currentAngle = g_animation.targetAngle;
    }
    
    // Tính tỉ lệ hoàn thành (0.0 -> 1.0)
    float progress = (g_animation.targetAngle > 0.0f)
        ? (g_animation.currentAngle / g_animation.targetAngle)
        : 1.0f;
    if (progress > 1.0f) {
        progress = 1.0f;
    }
    
    // Áp dụng easing để có chuyển động mượt mà
    g_animation.displayAngle = easeInOutCubic(progress) * g_animation.targetAngle;
    
    // Kiểm tra xem đã hoàn thành chưa
    if (g_animation.currentAngle >= g_animation.targetAngle - 0.0001f) {
        // Lưu thông tin trước khi reset
        Face finishedFace = g_animation.face;
        bool finishedDir = g_animation.clockwise;
        bool finishedWasScramble = g_animation.isScrambleMove;
        
        // Thực hiện xoay logic (cập nhật màu sắc của các mảnh)
        rotateFace(finishedFace, finishedDir);
        
        // Reset trạng thái animation
        g_animation.isActive = false;
        g_animation.isScrambleMove = false;
        g_animation.currentAngle = 0.0f;
        g_animation.displayAngle = 0.0f;
        
        // Xóa danh sách mảnh bị ảnh hưởng
        for (int i = 0; i < 9; i++) {
            g_animation.affectedIndices[i] = -1;
        }
        if (g_logFile != NULL) {
            const char* faceNames[] = {"FRONT", "BACK", "LEFT", "RIGHT", "UP", "DOWN"};
            double tsMs = getLogTimestampMs();
            fprintf(g_logFile, "[%010.3f ms] ANIM END %s %s | queue=%d\n",
                    tsMs,
                    faceNames[finishedFace],
                    finishedDir ? "CW" : "CCW",
                    g_moveQueue.count);
            fflush(g_logFile);
        }
        // Xử lý hoàn thành nước trộn (nếu có)
        handleScrambleMoveCompletion(finishedWasScramble);
        
        // Lấy nước đi tiếp theo từ hàng đợi (nếu có)
        Face nextFace;
        bool nextDir;
        bool nextIsScramble = false;
        if (dequeueQueuedMove(nextFace, nextDir, nextIsScramble)) {
            // Bắt đầu animation tiếp theo
            startRotation(nextFace, nextDir, nextIsScramble);
        }
    }
    
    // Yêu cầu vẽ lại màn hình
    glutPostRedisplay;
}

// Hàm callback idle - được gọi liên tục khi chương trình rảnh
// Dùng để cập nhật animation và timer
void idle() {
    // Lấy thời gian hiện tại (milliseconds kể từ khi chương trình bắt đầu)
    int currentTime = glutGet(GLUT_ELAPSED_TIME);
    
    // Khởi tạo lần đầu
    if (g_lastTimeMs == 0) {
        g_lastTimeMs = currentTime;
    }
    
    // Tính thời gian giữa 2 frame (chuyển sang giây)
    float deltaTime = (float)(currentTime - g_lastTimeMs) / 1000.0f;
    
    // Giới hạn deltaTime để tránh nhảy frame quá lớn
    if (deltaTime > 0.1f) {
        deltaTime = 0.1f;  // Tối đa 100ms
    } else if (deltaTime < 0.0f) {
        deltaTime = 0.0f;
    }
    
    // Lưu thời gian hiện tại cho frame tiếp theo
    g_lastTimeMs = currentTime;
    
    // Cập nhật animation và timer
    updateAnimation(deltaTime);
    updateTimer();
}
