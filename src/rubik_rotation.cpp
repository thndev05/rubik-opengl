#include "rubik_rotation.h"
#include "rubik_state.h"
#include "rubik_constants.h"
#include "rubik_input.h"
#include <cmath>
#include <cstring>
#include <cstdio>

/**
 * Trả về mặt đối diện của một mặt cho trước trên khối Rubik.
 * Hàm này rất hữu ích khi cần xác định mặt phía sau hoặc hướng ngược lại.
 * 
 * @param face Mặt đầu vào (ví dụ: FRONT).
 * @return Mặt đối diện (ví dụ: BACK).
 * 
 * Quy tắc đối diện:
 * - FRONT (Trước) <-> BACK (Sau)
 * - LEFT (Trái) <-> RIGHT (Phải)
 * - UP (Trên) <-> DOWN (Dưới)
 */
Face getOppositeFace(Face face) {
    switch (face) {
        case FRONT: return BACK;
        case BACK: return FRONT;
        case LEFT: return RIGHT;
        case RIGHT: return LEFT;
        case UP: return DOWN;
        case DOWN: return UP;
        default: return FRONT; // Mặc định trả về FRONT nếu giá trị không hợp lệ
    }
}

/**
 * Chuyển đổi từ biểu diễn trục-góc (axis-angle) sang ma trận xoay 3x3.
 * Hàm này sử dụng công thức Rodrigues' rotation để tạo ra ma trận xoay.
 * Ma trận này sau đó có thể dùng để nhân với vector để thực hiện phép xoay.
 * 
 * @param m Ma trận kết quả 3x3 sẽ chứa các giá trị tính toán được.
 * @param axis Vector trục xoay [x, y, z]. Không nhất thiết phải chuẩn hóa trước.
 * @param angleDegrees Góc xoay tính bằng độ (degrees).
 */
void axisAngleToMatrix(float m[3][3], const float axis[3], float angleDegrees) {
    // 1. Chuyển đổi góc từ độ sang radian để dùng hàm lượng giác
    float angleRad = angleDegrees * 3.14159f / 180.0f;
    float c = cos(angleRad);  // cos(theta)
    float s = sin(angleRad);  // sin(theta)
    float t = 1.0f - c;       // 1 - cos(theta), phần bù của cos
    
    // 2. Chuẩn hóa vector trục xoay (đảm bảo độ dài = 1)
    // Tính độ dài vector: sqrt(x^2 + y^2 + z^2)
    float length = sqrt(axis[0] * axis[0] + axis[1] * axis[1] + axis[2] * axis[2]);
    float x, y, z;
    if (length > 0.0001f) {
        // Chia từng thành phần cho độ dài để chuẩn hóa
        x = axis[0] / length;
        y = axis[1] / length;
        z = axis[2] / length;
    } else {
        // Nếu vector quá ngắn (gần bằng 0), giữ nguyên để tránh chia cho 0
        x = axis[0];
        y = axis[1];
        z = axis[2];
    }
    
    // 3. Áp dụng công thức Rodrigues' rotation matrix
    // Hàng 0
    m[0][0] = t * x * x + c;
    m[0][1] = t * x * y - s * z;
    m[0][2] = t * x * z + s * y;
    
    // Hàng 1
    m[1][0] = t * x * y + s * z;
    m[1][1] = t * y * y + c;
    m[1][2] = t * y * z - s * x;
    
    // Hàng 2
    m[2][0] = t * x * z - s * y;
    m[2][1] = t * y * z + s * x;
    m[2][2] = t * z * z + c;
}

/**
 * Nhân hai ma trận 3x3 với nhau.
 * Phép nhân ma trận không có tính giao hoán (A*B != B*A).
 * 
 * @param result Ma trận 3x3 chứa kết quả (result = a * b).
 * @param a Ma trận thứ nhất (bên trái).
 * @param b Ma trận thứ hai (bên phải).
 */
void matrixMultiply(float result[3][3], const float a[3][3], const float b[3][3]) {
    int i, j, k;
    // Duyệt qua từng hàng của ma trận kết quả
    for (i = 0; i < 3; i++) {
        // Duyệt qua từng cột của ma trận kết quả
        for (j = 0; j < 3; j++) {
            result[i][j] = 0.0f;
            // Tính tích vô hướng của hàng i (ma trận a) và cột j (ma trận b)
            for (k = 0; k < 3; k++) {
                result[i][j] += a[i][k] * b[k][j];
            }
        }
    }
}

/**
 * Xoay một vector 3D quanh một trục bất kỳ một góc nhất định.
 * Hàm này kết hợp việc tạo ma trận xoay và nhân ma trận với vector.
 * 
 * @param axis Trục xoay [x, y, z].
 * @param angleDegrees Góc xoay (độ).
 * @param x, y, z Toạ độ vector đầu vào (tham chiếu - sẽ được cập nhật trực tiếp thành giá trị mới).
 */
void rotateVectorAroundAxis(const float axis[3], float angleDegrees,
                            float& x, float& y, float& z) {
    // Nếu góc xoay quá nhỏ (gần bằng 0), không cần làm gì để tiết kiệm chi phí tính toán
    if (fabs(angleDegrees) < 0.0001f) {
        return;
    }
    
    // 1. Tạo ma trận xoay từ trục và góc
    float rot[3][3];
    axisAngleToMatrix(rot, axis, angleDegrees);
    
    // 2. Nhân ma trận với vector (Matrix-Vector Multiplication)
    // V_new = M * V_old
    float rx = rot[0][0] * x + rot[0][1] * y + rot[0][2] * z;
    float ry = rot[1][0] * x + rot[1][1] * y + rot[1][2] * z;
    float rz = rot[2][0] * x + rot[2][1] * y + rot[2][2] * z;
    
    // 3. Cập nhật kết quả vào các biến tham chiếu
    x = rx;
    y = ry;
    z = rz;
}

/**
 * Xoay toạ độ rời rạc (integer) của một mảnh Rubik trong lưới 3x3x3.
 * Toạ độ lưới của Rubik nằm trong khoảng {-1, 0, +1} trên mỗi trục.
 * Hàm này tính toán vị trí mới của mảnh sau khi xoay 90 độ.
 * 
 * @param axis Trục xoay: 0=X (Trái/Phải), 1=Y (Trên/Dưới), 2=Z (Trước/Sau).
 * @param axisSign Chiều của trục: 1 hoặc -1. Ví dụ: Mặt Front là Z=1, Back là Z=-1.
 * @param clockwise Hướng xoay: true (cùng chiều kim đồng hồ), false (ngược chiều).
 * @param x, y, z Toạ độ hiện tại của mảnh.
 * @param rx, ry, rz Toạ độ mới sau khi xoay (tham chiếu đầu ra).
 */
void rotateCoordinates(int axis, int axisSign, bool clockwise,
                       int x, int y, int z,
                       int& rx, int& ry, int& rz) {
    // Xác định dấu của góc xoay dựa trên chiều kim đồng hồ và chiều trục
    // Quy tắc bàn tay phải: ngón cái chỉ theo trục dương, ngón tay nắm lại là chiều dương (CCW)
    // Ở đây clockwise (CW) thường tương ứng với góc âm trong hệ toạ độ chuẩn
    int angleSign = clockwise ? -1 : 1;
    angleSign *= axisSign;
    
    switch (axis) {
        case 0: // Xoay quanh trục X (giữ nguyên x)
            rx = x;
            // Phép xoay 2D trên mặt phẳng YZ
            if (angleSign > 0) {
                ry = -z;
                rz = y;
            } else {
                ry = z;
                rz = -y;
            }
            break;
            
        case 1: // Xoay quanh trục Y (giữ nguyên y)
            ry = y;
            // Phép xoay 2D trên mặt phẳng ZX
            if (angleSign > 0) {
                rz = -x;
                rx = z;
            } else {
                rz = x;
                rx = -z;
            }
            break;
            
        case 2: // Xoay quanh trục Z (giữ nguyên z)
            rz = z;
            // Phép xoay 2D trên mặt phẳng XY
            if (angleSign > 0) {
                rx = -y;
                ry = x;
            } else {
                rx = y;
                ry = -x;
            }
            break;
            
        default:
            // Trường hợp lỗi, giữ nguyên toạ độ
            rx = x;
            ry = y;
            rz = z;
            break;
    }
}

/**
 * Xoay hướng (orientation) của một mảnh cụ thể.
 * Khi một mảnh di chuyển sang vị trí mới, các mặt màu của nó cũng bị xoay theo.
 * Ví dụ: Khi xoay mặt phải, mặt trên của mảnh góc sẽ chuyển sang mặt sau.
 * 
 * @param pieceIndex Chỉ số của mảnh trong mảng g_rubikCube.pieces.
 * @param axis Trục xoay (0=X, 1=Y, 2=Z).
 * @param clockwise Hướng xoay.
 */
void rotatePieceOrientation(int pieceIndex, int axis, bool clockwise) {
    CubePiece* p = &g_rubikCube.pieces[pieceIndex];
    float temp[3]; // Biến tạm để hoán đổi màu
    int i;
    
    // Các chỉ số màu trong mảng colors:
    // 0: Front, 1: Back, 2: Left, 3: Right, 4: Up, 5: Down
    
    if (axis == 2) {  // Xoay quanh trục Z (Mặt Front/Back)
        // Các mặt bị ảnh hưởng: Up(4), Left(2), Down(5), Right(3)
        // Front(0) và Back(1) không đổi màu (chỉ xoay tại chỗ)
        if (clockwise) {
            // Chu trình: Up -> Right -> Down -> Left -> Up
            // Lưu ý: Code dưới đây thực hiện hoán đổi giá trị màu
            for (i = 0; i < 3; i++) temp[i] = p->colors[4][i];      // Lưu Up
            for (i = 0; i < 3; i++) p->colors[4][i] = p->colors[2][i]; // Left -> Up
            for (i = 0; i < 3; i++) p->colors[2][i] = p->colors[5][i]; // Down -> Left
            for (i = 0; i < 3; i++) p->colors[5][i] = p->colors[3][i]; // Right -> Down
            for (i = 0; i < 3; i++) p->colors[3][i] = temp[i];      // Up -> Right
        } else {
            // Chu trình ngược lại
            for (i = 0; i < 3; i++) temp[i] = p->colors[4][i];
            for (i = 0; i < 3; i++) p->colors[4][i] = p->colors[3][i];
            for (i = 0; i < 3; i++) p->colors[3][i] = p->colors[5][i];
            for (i = 0; i < 3; i++) p->colors[5][i] = p->colors[2][i];
            for (i = 0; i < 3; i++) p->colors[2][i] = temp[i];
        }
    } else if (axis == 0) {  // Xoay quanh trục X (Mặt Left/Right)
        // Các mặt bị ảnh hưởng: Front(0), Up(4), Back(1), Down(5)
        if (clockwise) {
            for (i = 0; i < 3; i++) temp[i] = p->colors[0][i];
            for (i = 0; i < 3; i++) p->colors[0][i] = p->colors[5][i]; // Down -> Front
            for (i = 0; i < 3; i++) p->colors[5][i] = p->colors[1][i]; // Back -> Down
            for (i = 0; i < 3; i++) p->colors[1][i] = p->colors[4][i]; // Up -> Back
            for (i = 0; i < 3; i++) p->colors[4][i] = temp[i];      // Front -> Up
        } else {
            for (i = 0; i < 3; i++) temp[i] = p->colors[0][i];
            for (i = 0; i < 3; i++) p->colors[0][i] = p->colors[4][i];
            for (i = 0; i < 3; i++) p->colors[4][i] = p->colors[1][i];
            for (i = 0; i < 3; i++) p->colors[1][i] = p->colors[5][i];
            for (i = 0; i < 3; i++) p->colors[5][i] = temp[i];
        }
    } else if (axis == 1) {  // Xoay quanh trục Y (Mặt Up/Down)
        // Các mặt bị ảnh hưởng: Front(0), Right(3), Back(1), Left(2)
        if (clockwise) {
            for (i = 0; i < 3; i++) temp[i] = p->colors[0][i];
            for (i = 0; i < 3; i++) p->colors[0][i] = p->colors[2][i]; // Left -> Front
            for (i = 0; i < 3; i++) p->colors[2][i] = p->colors[1][i]; // Back -> Left
            for (i = 0; i < 3; i++) p->colors[1][i] = p->colors[3][i]; // Right -> Back
            for (i = 0; i < 3; i++) p->colors[3][i] = temp[i];      // Front -> Right
        } else {
            for (i = 0; i < 3; i++) temp[i] = p->colors[0][i];
            for (i = 0; i < 3; i++) p->colors[0][i] = p->colors[3][i];
            for (i = 0; i < 3; i++) p->colors[3][i] = p->colors[1][i];
            for (i = 0; i < 3; i++) p->colors[1][i] = p->colors[2][i];
            for (i = 0; i < 3; i++) p->colors[2][i] = temp[i];
        }
    }
}

/**
 * Cập nhật logic vị trí và màu sắc của các mảnh sau khi xoay một mặt.
 * Đây là hàm phức tạp nhất, chịu trách nhiệm cập nhật trạng thái Rubik.
 * 
 * @param face Mặt được xoay.
 * @param clockwise Hướng xoay.
 */
void rotatePositions(int face, bool clockwise) {
    // 1. Lấy danh sách 9 mảnh thuộc mặt đang xoay
    int indices[9];
    getFaceIndices(face, indices);
    
    // 2. Sao lưu màu sắc hiện tại của 9 mảnh này
    // Vì ta sẽ ghi đè màu sắc, cần bản sao để tham chiếu
    float backupColors[9][6][3];
    int i, f, c;
    for (i = 0; i < 9; i++) {
        for (f = 0; f < 6; f++) {
            for (c = 0; c < 3; c++) {
                backupColors[i][f][c] = g_rubikCube.pieces[indices[i]].colors[f][c];
            }
        }
    }
    
    // 3. Xác định trục xoay và chiều của trục dựa trên mặt
    int rotationAxis;
    int axisSign = 1;
    switch (face) {
        case FRONT:
        case BACK:
            rotationAxis = 2; // Trục Z
            axisSign = (face == FRONT) ? 1 : -1;
            break;
        case LEFT:
        case RIGHT:
            rotationAxis = 0; // Trục X
            axisSign = (face == RIGHT) ? 1 : -1;
            break;
        case UP:
        case DOWN:
            rotationAxis = 1; // Trục Y
            axisSign = (face == UP) ? 1 : -1;
            break;
        default:
            rotationAxis = 2;
            break;
    }
    
    // 4. Tính toán sự hoán đổi vị trí (Permutation)
    // mapping[destSlot] = srcSlot: Mảnh ở vị trí srcSlot sẽ chuyển đến destSlot
    int mapping[9];
    int keyToSlot[27]; // Bản đồ ngược từ Key vị trí -> Chỉ số trong mảng indices (0-8)
    
    // Khởi tạo bản đồ ngược
    for (i = 0; i < 27; i++) {
        keyToSlot[i] = -1;
    }
    for (i = 0; i < 9; i++) {
        const CubePiece& piece = g_rubikCube.pieces[indices[i]];
        int key = encodePositionKey(piece.position[0], piece.position[1], piece.position[2]);
        keyToSlot[key] = i;
    }
    
    // Tính toán vị trí đích cho từng mảnh
    for (i = 0; i < 9; i++) {
        const CubePiece& piece = g_rubikCube.pieces[indices[i]];
        int rx, ry, rz;
        // Tính toạ độ mới sau khi xoay
        rotateCoordinates(rotationAxis, axisSign, clockwise,
                          piece.position[0], piece.position[1], piece.position[2],
                          rx, ry, rz);
        
        // Tìm xem vị trí mới này tương ứng với slot nào trong 9 slot của mặt
        int key = encodePositionKey(rx, ry, rz);
        int destSlot = keyToSlot[key];
        
        // Lưu lại: slot đích này sẽ nhận dữ liệu từ slot nguồn i
        mapping[destSlot] = i;
    }
    
    // 5. Cập nhật màu sắc cho các mảnh tại vị trí mới
    int j;
    int srcIdx;
    float temp[3];
    
    for (i = 0; i < 9; i++) {
        // Mảnh tại vị trí i (indices[i]) sẽ nhận màu từ mảnh tại vị trí mapping[i] (trong bản sao lưu)
        srcIdx = mapping[i];
        
        // Sao chép màu từ bản sao lưu
        for (f = 0; f < 6; f++) {
            for (c = 0; c < 3; c++) {
                g_rubikCube.pieces[indices[i]].colors[f][c] = backupColors[srcIdx][f][c];
            }
        }
        
        // 6. Xoay hướng màu (Orientation)
        // Mảnh trung tâm (i=4) không thay đổi hướng màu tương đối so với mặt
        if (i != 4) {
            CubePiece* p = &g_rubikCube.pieces[indices[i]];
            
            // Xác định chiều xoay hướng màu
            bool orientationClockwise = clockwise;
            if (axisSign < 0) {
                orientationClockwise = !orientationClockwise;
            }
            
            // Thực hiện hoán đổi màu các mặt của mảnh để phản ánh việc nó bị xoay
            // Logic tương tự như rotatePieceOrientation nhưng được viết inline ở đây
            if (orientationClockwise) {
                if (rotationAxis == 2) { // Z
                    for (j = 0; j < 3; j++) temp[j] = p->colors[4][j];
                    for (j = 0; j < 3; j++) p->colors[4][j] = p->colors[2][j];
                    for (j = 0; j < 3; j++) p->colors[2][j] = p->colors[5][j];
                    for (j = 0; j < 3; j++) p->colors[5][j] = p->colors[3][j];
                    for (j = 0; j < 3; j++) p->colors[3][j] = temp[j];
                } else if (rotationAxis == 0) { // X
                    for (j = 0; j < 3; j++) temp[j] = p->colors[0][j];
                    for (j = 0; j < 3; j++) p->colors[0][j] = p->colors[5][j];
                    for (j = 0; j < 3; j++) p->colors[5][j] = p->colors[1][j];
                    for (j = 0; j < 3; j++) p->colors[1][j] = p->colors[4][j];
                    for (j = 0; j < 3; j++) p->colors[4][j] = temp[j];
                } else if (rotationAxis == 1) { // Y
                    for (j = 0; j < 3; j++) temp[j] = p->colors[0][j];
                    for (j = 0; j < 3; j++) p->colors[0][j] = p->colors[3][j];
                    for (j = 0; j < 3; j++) p->colors[3][j] = p->colors[1][j];
                    for (j = 0; j < 3; j++) p->colors[1][j] = p->colors[2][j];
                    for (j = 0; j < 3; j++) p->colors[2][j] = temp[j];
                }
            } else {
                // Ngược chiều kim đồng hồ
                if (rotationAxis == 2) {
                    for (j = 0; j < 3; j++) temp[j] = p->colors[4][j];
                    for (j = 0; j < 3; j++) p->colors[4][j] = p->colors[3][j];
                    for (j = 0; j < 3; j++) p->colors[3][j] = p->colors[5][j];
                    for (j = 0; j < 3; j++) p->colors[5][j] = p->colors[2][j];
                    for (j = 0; j < 3; j++) p->colors[2][j] = temp[j];
                } else if (rotationAxis == 0) {
                    for (j = 0; j < 3; j++) temp[j] = p->colors[0][j];
                    for (j = 0; j < 3; j++) p->colors[0][j] = p->colors[4][j];
                    for (j = 0; j < 3; j++) p->colors[4][j] = p->colors[1][j];
                    for (j = 0; j < 3; j++) p->colors[1][j] = p->colors[5][j];
                    for (j = 0; j < 3; j++) p->colors[5][j] = temp[j];
                } else if (rotationAxis == 1) {
                    for (j = 0; j < 3; j++) temp[j] = p->colors[0][j];
                    for (j = 0; j < 3; j++) p->colors[0][j] = p->colors[2][j];
                    for (j = 0; j < 3; j++) p->colors[2][j] = p->colors[1][j];
                    for (j = 0; j < 3; j++) p->colors[1][j] = p->colors[3][j];
                    for (j = 0; j < 3; j++) p->colors[3][j] = temp[j];
                }
            }
        }
    }
    
    // Ghi log nếu cần
    if (g_logFile != NULL) {
        const char* faceNames[] = {"FRONT", "BACK", "LEFT", "RIGHT", "UP", "DOWN"};
        fprintf(g_logFile, "ROTATE %s %s: colors swapped, positions preserved\n",
                faceNames[face], clockwise ? "CW" : "CCW");
        fflush(g_logFile);
    }
}

/**
 * Hàm chính để xoay một mặt của Rubik's Cube.
 * Hàm này được gọi từ hệ thống xử lý input hoặc animation.
 * 
 * @param face Mặt cần xoay (FRONT, BACK, LEFT, RIGHT, UP, DOWN).
 * @param clockwise true = xuôi chiều kim đồng hồ, false = ngược chiều.
 */
void rotateFace(int face, bool clockwise) {
    int indices[9];
    // 1. Lấy danh sách 9 mảnh thuộc mặt này
    getFaceIndices(face, indices);
    
    // 2. Thực hiện xoay logic (cập nhật màu sắc và trạng thái)
    // Hàm rotatePositions sẽ xử lý việc hoán đổi màu giữa các mảnh
    rotatePositions(face, clockwise);
    
    // 3. Ghi log để debug nếu cần
    if (g_logFile != NULL) {
        const char* faceNames[] = {"FRONT", "BACK", "LEFT", "RIGHT", "UP", "DOWN"};
        double tsMs = getLogTimestampMs();
        fprintf(g_logFile, "[%010.3f ms] ROTATE %s %s: pieces [%d,%d,%d,%d,%d,%d,%d,%d,%d]\n", 
            tsMs,
            faceNames[face], clockwise ? "CW" : "CCW",
            indices[0], indices[1], indices[2], indices[3], indices[4],
            indices[5], indices[6], indices[7], indices[8]);
        fflush(g_logFile);
    }
}

/**
 * Xác định mặt tuyệt đối của khối Rubik dựa trên góc nhìn camera.
 * Khi người dùng xoay camera, khái niệm "mặt trước" (relative) thay đổi.
 * Hàm này ánh xạ từ mặt tương đối (theo góc nhìn) sang mặt tuyệt đối (cố định trên khối).
 * 
 * @param relativeFace Mặt tương đối (0=Front theo góc nhìn, v.v.).
 * @return Face Mặt tuyệt đối tương ứng trên khối Rubik.
 */
Face getAbsoluteFace(int relativeFace) {
    // 1. Tính toán ánh xạ dựa trên góc quay camera hiện tại
    ViewFaceMapping mapping;
    computeViewFaceMapping(mapping);
    
    Face selected = mapping.front;
    // 2. Ánh xạ từ input (relative) sang output (absolute)
    switch (relativeFace) {
        case 0: selected = mapping.front; break;
        case 1: selected = mapping.up; break;
        case 2: selected = mapping.right; break;
        case 3: selected = mapping.left; break;
        case 4: selected = mapping.down; break;
        case 5: selected = mapping.back; break;
        default: selected = mapping.front; break;
    }
    
    // 3. Ghi log để debug
    if (g_logFile != NULL) {
        const char* faceNames[] = {"FRONT", "BACK", "LEFT", "RIGHT", "UP", "DOWN"};
        double tsMs = getLogTimestampMs();
        extern float cameraAngleX, cameraAngleY;
        fprintf(g_logFile,
            "[%010.3f ms] REL FACE %d -> %s (front=%s up=%s right=%s) angles(X=%.1f,Y=%.1f)\n",
            tsMs,
            relativeFace,
            faceNames[selected],
            faceNames[mapping.front],
            faceNames[mapping.up],
            faceNames[mapping.right],
            cameraAngleX,
            cameraAngleY);
        fflush(g_logFile);
    }
    return selected;
}
