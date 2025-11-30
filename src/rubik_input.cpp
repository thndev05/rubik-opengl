#include "rubik_input.h"
#include "rubik_state.h"
#include "rubik_rotation.h"
#include "rubik_animation.h"
#include "rubik_constants.h"
#include <GL/glut.h>
#include <cstdio>
#include <cctype>
#include <cmath>

// Góc xoay camera (độ)
float cameraAngleX = 0.0f;  // Xoay theo trục ngang (pitch)
float cameraAngleY = 0.0f;  // Xoay theo trục dọc (yaw)

// Trạng thái kéo chuột
bool isDragging = false;
int lastMouseX = 0;  // Vị trí chuột cuối cùng
int lastMouseY = 0;

// Mặt hiện tại đang hướng về phía người dùng
Face currentFrontFace = FRONT;

// Trục xoay động theo góc nhìn
float verticalAxis[3] = {0.0f, 0.0f, 0.0f};    // Trục dọc để xoay lên/xuống
float horizontalAxis[3] = {0.0f, 0.0f, 0.0f};  // Trục ngang để xoay trái/phải

// Cập nhật các trục xoay camera dựa trên mặt hiện đang hướng về phía người dùng
// Điều này cho phép camera xoay "tự nhiên" theo hướng nhìn hiện tại
void updateRotationAxes() {
    // Reset các trục về 0
    verticalAxis[0] = 0.0f;
    verticalAxis[1] = 0.0f;
    verticalAxis[2] = 0.0f;
    horizontalAxis[0] = 0.0f;
    horizontalAxis[1] = 0.0f;
    horizontalAxis[2] = 0.0f;
    
    // Thiết lập trục xoay dựa trên mặt trước hiện tại
    switch (currentFrontFace) {
        case FRONT:  // Nhìn từ phía trước
            verticalAxis[0] = 1.0f;    // Xoay quanh trục X
            horizontalAxis[1] = 1.0f;  // Xoay quanh trục Y
            break;
        case RIGHT:  // Nhìn từ bên phải
            verticalAxis[2] = 1.0f;    // Xoay quanh trục Z
            horizontalAxis[0] = 1.0f;  // Xoay quanh trục X
            break;
        case BACK:   // Nhìn từ phía sau
            verticalAxis[0] = -1.0f;   // Xoay quanh trục X (ngược chiều)
            horizontalAxis[1] = -1.0f; // Xoay quanh trục Y (ngược chiều)
            break;
        case LEFT:   // Nhìn từ bên trái
            verticalAxis[2] = -1.0f;   // Xoay quanh trục Z (ngược chiều)
            horizontalAxis[0] = -1.0f; // Xoay quanh trục X (ngược chiều)
            break;
        case UP:     // Nhìn từ phía trên
            verticalAxis[1] = 1.0f;    // Xoay quanh trục Y
            horizontalAxis[2] = 1.0f;  // Xoay quanh trục Z
            break;
        case DOWN:   // Nhìn từ phía dưới
            verticalAxis[1] = -1.0f;   // Xoay quanh trục Y (ngược chiều)
            horizontalAxis[2] = -1.0f; // Xoay quanh trục Z (ngược chiều)
            break;
    }
    
    if (g_logFile != NULL) {
        const char* faceNames[] = {"FRONT", "BACK", "LEFT", "RIGHT", "UP", "DOWN"};
        fprintf(g_logFile, "FACE CHANGE: %s | verticalAxis=[%.1f,%.1f,%.1f] horizontalAxis=[%.1f,%.1f,%.1f]\n",
                faceNames[currentFrontFace],
                verticalAxis[0], verticalAxis[1], verticalAxis[2],
                horizontalAxis[0], horizontalAxis[1], horizontalAxis[2]);
        fflush(g_logFile);
    }
}

void resetRotationAngles() {
    cameraAngleX = 0.0f;
    cameraAngleY = 0.0f;
}

// Áp dụng góc xoay camera hiện tại lên một vector
// Dùng để chuyển đổi tọa độ từ không gian thế giới sang không gian camera
void applyCurrentViewRotation(float& x, float& y, float& z) {
    rotateVectorAroundAxis(verticalAxis, cameraAngleX, x, y, z);
    rotateVectorAroundAxis(horizontalAxis, cameraAngleY, x, y, z);
}

// Tính toán ánh xạ mặt theo góc nhìn hiện tại
// Xác định mặt nào của cube đang hướng về các hướng front/back/left/right/up/down
// từ góc nhìn của camera hiện tại
void computeViewFaceMapping(ViewFaceMapping& mapping) {
    // Vector pháp tuyến của 6 mặt cube trong không gian thế giới
    const float normals[6][3] = {
        {0.0f, 0.0f, 1.0f},   // FRONT
        {0.0f, 0.0f, -1.0f},  // BACK
        {-1.0f, 0.0f, 0.0f},  // LEFT
        {1.0f, 0.0f, 0.0f},   // RIGHT
        {0.0f, 1.0f, 0.0f},   // UP
        {0.0f, -1.0f, 0.0f}   // DOWN
    };
    const Face faces[6] = {FRONT, BACK, LEFT, RIGHT, UP, DOWN};
    
    // Xoay tất cả các vector pháp tuyến theo góc nhìn camera hiện tại
    float rotated[6][3];
    for (int i = 0; i < 6; i++) {
        rotated[i][0] = normals[i][0];
        rotated[i][1] = normals[i][1];
        rotated[i][2] = normals[i][2];
        applyCurrentViewRotation(rotated[i][0], rotated[i][1], rotated[i][2]);
    }
    
    // Biến để lưu kết quả tìm kiếm
    float bestFrontDot = -1000.0f;
    float bestUpDot = -1000.0f;
    int frontIdx = 0;
    int upIdx = 4;
    
    // Vector hướng nhìn và hướng trên từ góc nhìn camera
    const float viewFront[3] = {0.0f, 0.0f, 1.0f};  // Hướng về phía trước màn hình
    const float viewUp[3] = {0.0f, 1.0f, 0.0f};     // Hướng lên trên
    
    // Tìm mặt cube nào đang hướng về phía camera nhiều nhất (dot product lớn nhất)
    for (int i = 0; i < 6; i++) {
        float dotFront = rotated[i][0] * viewFront[0] + rotated[i][1] * viewFront[1] + rotated[i][2] * viewFront[2];
        if (dotFront > bestFrontDot) {
            bestFrontDot = dotFront;
            frontIdx = i;
        }
    }
    // Tìm mặt cube nào đang hướng lên trên nhiều nhất
    // (loại trừ mặt front và back vì chúng không thể là up)
    for (int i = 0; i < 6; i++) {
        if (i == frontIdx || faces[i] == getOppositeFace(faces[frontIdx])) {
            continue;  // Bỏ qua mặt front và back
        }
        float dotUp = rotated[i][0] * viewUp[0] + rotated[i][1] * viewUp[1] + rotated[i][2] * viewUp[2];
        if (dotUp > bestUpDot) {
            bestUpDot = dotUp;
            upIdx = i;
        }
    }
    float frontVec[3] = {rotated[frontIdx][0], rotated[frontIdx][1], rotated[frontIdx][2]};
    float upVec[3] = {rotated[upIdx][0], rotated[upIdx][1], rotated[upIdx][2]};
    float frontDotUp = frontVec[0] * upVec[0] + frontVec[1] * upVec[1] + frontVec[2] * upVec[2];
    if (fabs(frontDotUp) > 0.9f) {
        bestUpDot = -1000.0f;
        for (int i = 0; i < 6; i++) {
            if (i == frontIdx || faces[i] == getOppositeFace(faces[frontIdx])) {
                continue;
            }
            float dotUp = rotated[i][0] * viewUp[0] + rotated[i][1] * viewUp[1] + rotated[i][2] * viewUp[2];
            if (dotUp > bestUpDot && fabs(rotated[i][0] * frontVec[0] + rotated[i][1] * frontVec[1] + rotated[i][2] * frontVec[2]) < 0.1f) {
                bestUpDot = dotUp;
                upIdx = i;
                upVec[0] = rotated[i][0];
                upVec[1] = rotated[i][1];
                upVec[2] = rotated[i][2];
            }
        }
    }
    // Tính vector right bằng tích có hướng: right = up × front
    // Điều này đảm bảo right vuông góc với cả up và front
    float rightVec[3];
    rightVec[0] = upVec[1] * frontVec[2] - upVec[2] * frontVec[1];
    rightVec[1] = upVec[2] * frontVec[0] - upVec[0] * frontVec[2];
    rightVec[2] = upVec[0] * frontVec[1] - upVec[1] * frontVec[0];
    
    // Chuẩn hóa vector right
    float rightLen = sqrt(rightVec[0] * rightVec[0] + rightVec[1] * rightVec[1] + rightVec[2] * rightVec[2]);
    if (rightLen > 0.0001f) {
        rightVec[0] /= rightLen;
        rightVec[1] /= rightLen;
        rightVec[2] /= rightLen;
    } else {
        rightVec[0] = 1.0f;
        rightVec[1] = 0.0f;
        rightVec[2] = 0.0f;
    }
    int rightIdx = 3;
    float bestRightDot = -1000.0f;
    for (int i = 0; i < 6; i++) {
        float dotRight = rotated[i][0] * rightVec[0] + rotated[i][1] * rightVec[1] + rotated[i][2] * rightVec[2];
        if (dotRight > bestRightDot) {
            bestRightDot = dotRight;
            rightIdx = i;
        }
    }
    mapping.front = faces[frontIdx];
    mapping.back = getOppositeFace(mapping.front);
    mapping.up = faces[upIdx];
    mapping.down = getOppositeFace(mapping.up);
    mapping.right = faces[rightIdx];
    mapping.left = getOppositeFace(mapping.right);
}

// Thực hiện xoay mặt dựa trên hướng tương đối (relative)
// relativeFace: 0=front, 1=up, 2=right, 3=left, 4=down, 5=back (theo góc nhìn)
// Chuyển đổi sang mặt tuyệt đối (absolute) rồi thực hiện xoay
bool performRelativeFaceTurn(int relativeFace, bool clockwise) {
    Face absoluteFace = getAbsoluteFace(relativeFace);
    startRotation(absoluteFace, clockwise);
    return true;
}

// Callback xử lý sự kiện chuột
// Xử lý click và drag để xoay camera
void mouse(int button, int state, int x, int y) {
    if (g_logFile != NULL) {
        fprintf(g_logFile, "MOUSE EVENT: button=%d state=%d x=%d y=%d\n", button, state, x, y);
        fflush(g_logFile);
    }
    
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            if (g_logFile != NULL) {
                fprintf(g_logFile, "*** DRAG START ***\n");
                fflush(g_logFile);
            }
            isDragging = true;
            lastMouseX = x;
            lastMouseY = y;
        } else if (state == GLUT_UP) {
            if (g_logFile != NULL) {
                fprintf(g_logFile, "*** DRAG END ***\n");
                fflush(g_logFile);
            }
            isDragging = false;
        }
    }
    glutPostRedisplay();
}

// Callback xử lý chuyển động chuột khi đang kéo
// Cập nhật góc xoay camera dựa trên độ dịch chuyển chuột
void motion(int x, int y) {
    // Chỉ xử lý khi đang kéo chuột
    if (!isDragging) {
        return;
    }
    
    // Tính độ dịch chuyển từ vị trí trước đó
    int dx = x - lastMouseX;
    int dy = y - lastMouseY;
    
    float yawDelta = (float)dx * ROTATION_SENSITIVITY;
    float pitchDelta = (float)(y - lastMouseY) * ROTATION_SENSITIVITY;
    
    if (g_logFile != NULL) {
        fprintf(g_logFile, "MOUSE: x=%d y=%d | dx=%d dy=%d | yawDelta=%.1f pitchDelta=%.1f\n", 
                x, y, dx, dy, yawDelta, pitchDelta);
        fflush(g_logFile);
    }
    
    cameraAngleY += yawDelta;
    cameraAngleX += pitchDelta;
    
    if (g_logFile != NULL) {
        fprintf(g_logFile, "  → Updated angles: X=%.1f Y=%.1f\n", cameraAngleX, cameraAngleY);
        fflush(g_logFile);
    }
    
    lastMouseX = x;
    lastMouseY = y;
    glutPostRedisplay();
}

// Callback xử lý phím bấm
// Điều khiển xoay các mặt cube và các chức năng khác
void keyboard(unsigned char key, int /* x */, int /* y */) {
    // Kiểm tra phím Shift có được giữ không
    int modifiers = glutGetModifiers();
    bool shiftDown = (modifiers & GLUT_ACTIVE_SHIFT) != 0;
    
    // Chuyển phím về chữ hoa để xử lý
    int keyUpper = toupper((unsigned char)key);
    
    // Theo dõi các phím quan trọng để tránh lặp lại
    bool trackKey = false;
    switch (keyUpper) {
        case 'F':
        case 'U':
        case 'R':
        case 'L':
        case 'D':
        case 'B':
        case 'S':
            trackKey = true;
            break;
        default:
            break;
    }
    if (key == ' ') {
        trackKey = true;
        keyUpper = ' ';
    }
    if (trackKey) {
        unsigned char idx = (unsigned char)keyUpper;
        if (g_keyHeld[idx]) {
            return;
        }
        g_keyHeld[idx] = true;
    }
    Face newFace = currentFrontFace;
    bool faceChanged = false;
    
    switch (keyUpper) {
        case ' ':  // Phím Space: Reset cube về trạng thái đã giải
            resetCube();
            glutPostRedisplay();
            return;
            
        case 'S':  // Phím S: Trộn cube (20 bước ngẫu nhiên)
            shuffleCube(20);
            glutPostRedisplay();
            return;
            
        case 'F':  // Phím F: Xoay mặt Front (Shift+F = ngược chiều)
            performRelativeFaceTurn(0, !shiftDown);
            return;
            
        case 'U':  // Phím U: Xoay mặt Up (Shift+U = ngược chiều)
            performRelativeFaceTurn(1, !shiftDown);
            return;
            
        case 'R':  // Phím R: Xoay mặt Right (Shift+R = ngược chiều)
            performRelativeFaceTurn(2, !shiftDown);
            return;
            
        case 'L':  // Phím L: Xoay mặt Left (Shift+L = ngược chiều)
            performRelativeFaceTurn(3, !shiftDown);
            return;
            
        case 'D':  // Phím D: Xoay mặt Down (Shift+D = ngược chiều)
            performRelativeFaceTurn(4, !shiftDown);
            return;
            
        case 'B':  // Phím B: Xoay mặt Back (Shift+B = ngược chiều)
            performRelativeFaceTurn(5, !shiftDown);
            return;
            
        case 'f':
            newFace = FRONT;
            faceChanged = true;
            break;
            
        case 'r':
            newFace = RIGHT;
            faceChanged = true;
            break;
            
        case 'b':
            newFace = BACK;
            faceChanged = true;
            break;
            
        case 'l':
            newFace = LEFT;
            faceChanged = true;
            break;
            
        case 'u':
            newFace = UP;
            faceChanged = true;
            break;
            
        case 'd':
            newFace = DOWN;
            faceChanged = true;
            break;
            
        default:
            break;
    }
    
    switch (key) {
        case 'f':
            newFace = FRONT;
            faceChanged = true;
            break;
            
        case 'r':
            newFace = RIGHT;
            faceChanged = true;
            break;
            
        case 'b':
            newFace = BACK;
            faceChanged = true;
            break;
            
        case 'l':
            newFace = LEFT;
            faceChanged = true;
            break;
            
        case 'u':
            newFace = UP;
            faceChanged = true;
            break;
            
        case 'd':
            newFace = DOWN;
            faceChanged = true;
            break;
        default:
            break;
    }
    
    if (faceChanged) {
        currentFrontFace = newFace;
        updateRotationAxes();
        glutPostRedisplay();
    }
}

void keyboardUp(unsigned char key, int /* x */, int /* y */) {
    int keyUpper = toupper((unsigned char)key);
    if (keyUpper < 0 || keyUpper >= 256) {
        return;
    }
    g_keyHeld[(unsigned char)keyUpper] = false;
    if (key == ' ') {
        g_keyHeld[(unsigned char)' '] = false;
    }
}

// Callback xử lý phím đặc biệt (mũi tên, F1-F12, etc.)
// Dùng phím mũi tên để xoay camera
void keyboardSpecial(int key, int /* x */, int /* y */) {
    const float ROTATION_STEP = KEYBOARD_ROTATION_SPEED;
    const char* keyName = "";
    
    switch (key) {
        case GLUT_KEY_UP:     // Mũi tên lên: xoay camera lên
            cameraAngleX -= ROTATION_STEP;
            keyName = "UP";
            break;
            
        case GLUT_KEY_DOWN:   // Mũi tên xuống: xoay camera xuống
            cameraAngleX += ROTATION_STEP;
            keyName = "DOWN";
            break;
            
        case GLUT_KEY_LEFT:   // Mũi tên trái: xoay camera sang trái
            cameraAngleY -= ROTATION_STEP;
            keyName = "LEFT";
            break;
            
        case GLUT_KEY_RIGHT:  // Mũi tên phải: xoay camera sang phải
            cameraAngleY += ROTATION_STEP;
            keyName = "RIGHT";
            break;
            
        default:
            return;
    }
    
    if (g_logFile != NULL) {
        fprintf(g_logFile, "KEYBOARD: %s pressed | angleX=%.1f angleY=%.1f\n", keyName, cameraAngleX, cameraAngleY);
        fflush(g_logFile);
    }
    
    glutPostRedisplay();
}
