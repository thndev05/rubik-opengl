#include "rubik_state.h"
#include "rubik_constants.h"
#include "rubik_animation.h"
#include "rubik_timer.h"
#include <cstdio>
#include <ctime>
#include <cstring>
#include <cmath>
#include <cstdlib>

RubikCube g_rubikCube;
FILE* g_logFile = NULL;
clock_t g_logStartClock = 0;

void initLogFile() {
    g_logFile = fopen("rubik_debug.log", "w");
    if (g_logFile == NULL) {
        return;
    }
    g_logStartClock = clock();
    
    time_t rawTime;
    struct tm* timeInfo;
    char timeStr[80];
    
    time(&rawTime);
    timeInfo = localtime(&rawTime);
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", timeInfo);
    
    fprintf(g_logFile, "=== Nhật ký Debug Rubik's Cube ===\n");
    fprintf(g_logFile, "Bắt đầu: %s\n", timeStr);
    fprintf(g_logFile, "==============================\n\n");
    fflush(g_logFile);
}

double getLogTimestampMs() {
    if (g_logStartClock == 0) {
        return 0.0;
    }
    clock_t current = clock();
    return (double)(current - g_logStartClock) * 1000.0 / CLOCKS_PER_SEC;
}

void closeLogFile() {
    if (g_logFile != NULL) {
        fprintf(g_logFile, "\n=== Kết thúc nhật ký ===\n");
        fclose(g_logFile);
        g_logFile = NULL;
    }
}

/**
 * Chuyển đổi từ toạ độ 3D (i, j, k) sang chỉ số mảng 1D (0-26).
 * Hàm này giúp truy cập nhanh vào mảng pieces dựa trên vị trí không gian.
 * 
 * @param i Toạ độ X (Trái/Phải): -1, 0, 1
 * @param j Toạ độ Y (Dưới/Trên): -1, 0, 1
 * @param k Toạ độ Z (Sau/Trước): -1, 0, 1
 * @return Chỉ số trong mảng g_rubikCube.pieces (0-26).
 * 
 * Quy tắc sắp xếp trong mảng:
 * - Lớp Z=1 (Trước): 9 phần tử đầu (0-8)
 * - Lớp Z=0 (Giữa): 9 phần tử tiếp (9-17)
 * - Lớp Z=-1 (Sau): 9 phần tử cuối (18-26)
 * Trong mỗi lớp, sắp xếp theo hàng (Y) rồi đến cột (X).
 */
int positionToIndex(int i, int j, int k) {
    int kOffset = (k == 1) ? 0 : (k == 0) ? 9 : 18;  // Offset lớp Z
    int jOffset = (j + 1) * 3;  // Offset hàng Y (0, 3, 6)
    int iOffset = (i + 1);      // Offset cột X (0, 1, 2)
    return kOffset + jOffset + iOffset;
}

/**
 * Lấy danh sách chỉ số của 9 mảnh thuộc một mặt cụ thể.
 * Hàm này duyệt qua toạ độ không gian tương ứng với mặt đó để tìm các mảnh.
 * 
 * @param face Mặt cần lấy (FRONT, BACK, v.v.).
 * @param indices Mảng đầu ra chứa 9 chỉ số mảnh.
 */
void getFaceIndices(int face, int indices[9]) {
    int idx = 0;
    int i, j, k;
    
    switch (face) {
        case FRONT: // Mặt trước: Z = 1
            k = 1;
            for (j = -1; j <= 1; j++) {     // Duyệt từ dưới lên trên
                for (i = -1; i <= 1; i++) { // Duyệt từ trái sang phải
                    indices[idx++] = positionToIndex(i, j, k);
                }
            }
            break;
            
        case BACK: // Mặt sau: Z = -1
            k = -1;
            for (j = -1; j <= 1; j++) {
                for (i = -1; i <= 1; i++) {
                    indices[idx++] = positionToIndex(i, j, k);
                }
            }
            break;
            
        case LEFT: // Mặt trái: X = -1
            i = -1;
            for (j = -1; j <= 1; j++) {
                for (k = 1; k >= -1; k--) { // Duyệt từ trước ra sau
                    indices[idx++] = positionToIndex(i, j, k);
                }
            }
            break;
            
        case RIGHT: // Mặt phải: X = 1
            i = 1;
            for (j = -1; j <= 1; j++) {
                for (k = 1; k >= -1; k--) {
                    indices[idx++] = positionToIndex(i, j, k);
                }
            }
            break;
            
        case UP: // Mặt trên: Y = 1
            j = 1;
            for (k = 1; k >= -1; k--) {
                for (i = -1; i <= 1; i++) {
                    indices[idx++] = positionToIndex(i, j, k);
                }
            }
            break;
            
        case DOWN: // Mặt dưới: Y = -1
            j = -1;
            for (k = 1; k >= -1; k--) {
                for (i = -1; i <= 1; i++) {
                    indices[idx++] = positionToIndex(i, j, k);
                }
            }
            break;
    }
}

// Mã hóa vị trí 3D thành một key duy nhất
// Dùng để tra cứu nhanh vị trí mảnh
// x, y, z ∈ {-1, 0, 1} -> key ∈ [0, 26]
int encodePositionKey(int x, int y, int z) {
    return (x + 1) * 9 + (y + 1) * 3 + (z + 1);
}

/**
 * Khởi tạo trạng thái ban đầu cho Rubik's Cube (trạng thái đã giải).
 * Hàm này thiết lập vị trí, kích thước và màu sắc cho tất cả 27 mảnh.
 * 
 * Quy tắc màu chuẩn:
 * - Trước (Front): Đỏ
 * - Sau (Back): Cam
 * - Trái (Left): Xanh lá
 * - Phải (Right): Xanh dương
 * - Trên (Up): Trắng
 * - Dưới (Down): Vàng
 */
void initRubikCube() {
    g_rubikCube.pieceSize = PIECE_SIZE;
    g_rubikCube.gapSize = GAP_SIZE;
    
    int index = 0;
    // Duyệt qua tất cả 27 vị trí trong lưới 3x3x3
    // k: trục Z (trước->sau: 1, 0, -1)
    // j: trục Y (dưới->trên: -1, 0, 1)
    // i: trục X (trái->phải: -1, 0, 1)
    for (int k = 1; k >= -1; k--) {
        for (int j = -1; j <= 1; j++) {
            for (int i = -1; i <= 1; i++) {
                CubePiece& piece = g_rubikCube.pieces[index];
                
                // Lưu vị trí lưới ban đầu
                piece.position[0] = i;
                piece.position[1] = j;
                piece.position[2] = k;
                piece.isVisible = true;
                
                // Khởi tạo tất cả mặt bằng màu đen (mặt ẩn bên trong khối)
                for (int face = 0; face < 6; face++) {
                    piece.colors[face][0] = COLOR_BLACK[0];
                    piece.colors[face][1] = COLOR_BLACK[1];
                    piece.colors[face][2] = COLOR_BLACK[2];
                }
                
                // Gán màu cho các mặt ngoài (chỉ gán nếu mặt đó hướng ra ngoài)
                
                // Mặt trước (Z=1): Đỏ
                if (k == 1) {
                    piece.colors[0][0] = COLOR_RED[0];
                    piece.colors[0][1] = COLOR_RED[1];
                    piece.colors[0][2] = COLOR_RED[2];
                }
                
                // Mặt sau (Z=-1): Cam
                if (k == -1) {
                    piece.colors[1][0] = COLOR_ORANGE[0];
                    piece.colors[1][1] = COLOR_ORANGE[1];
                    piece.colors[1][2] = COLOR_ORANGE[2];
                }
                
                // Mặt trái (X=-1): Xanh lá
                if (i == -1) {
                    piece.colors[2][0] = COLOR_GREEN[0];
                    piece.colors[2][1] = COLOR_GREEN[1];
                    piece.colors[2][2] = COLOR_GREEN[2];
                }
                
                // Mặt phải (X=1): Xanh dương
                if (i == 1) {
                    piece.colors[3][0] = COLOR_BLUE[0];
                    piece.colors[3][1] = COLOR_BLUE[1];
                    piece.colors[3][2] = COLOR_BLUE[2];
                }
                
                // Mặt trên (Y=1): Trắng
                if (j == 1) {
                    piece.colors[4][0] = COLOR_WHITE[0];
                    piece.colors[4][1] = COLOR_WHITE[1];
                    piece.colors[4][2] = COLOR_WHITE[2];
                }
                
                // Mặt dưới (Y=-1): Vàng
                if (j == -1) {
                    piece.colors[5][0] = COLOR_YELLOW[0];
                    piece.colors[5][1] = COLOR_YELLOW[1];
                    piece.colors[5][2] = COLOR_YELLOW[2];
                }
                
                index++;
            }
        }
    }
    
    if (g_logFile != NULL) {
        fprintf(g_logFile, "Giai đoạn 2: Đã khởi tạo %d mảnh Rubik\n", 27);
        fflush(g_logFile);
    }
}

void resetCube() {
    cancelAnimationAndQueue();
    initRubikCube();
    extern int g_scrambleMovesPending;
    g_scrambleMovesPending = 0;
    resetTimerState();
    if (g_logFile != NULL) {
        fprintf(g_logFile, "RESET: Cube về trạng thái đã giải\n");
        fflush(g_logFile);
    }
}

void shuffleCube(int numMoves) {
    if (numMoves <= 0) {
        return;
    }
    resetTimerState();
    extern int g_scrambleMovesPending;
    g_scrambleMovesPending = numMoves;
    for (int i = 0; i < numMoves; i++) {
        Face face = static_cast<Face>(rand() % 6);
        bool clockwise = (rand() % 2) == 0;
        startRotation(face, clockwise, true);
    }
    if (g_logFile != NULL) {
        fprintf(g_logFile, "TRỘN: %d bước ngẫu nhiên đã xếp hàng\n", numMoves);
        fflush(g_logFile);
    }
}

/**
 * Kiểm tra xem khối Rubik đã được giải hoàn tất chưa.
 * Một khối được coi là giải xong nếu tất cả 9 ô trên mỗi mặt đều cùng màu.
 * 
 * @return true nếu đã giải, false nếu chưa.
 */
bool isCubeSolved() {
    const float tolerance = 0.05f;  // Dung sai so sánh màu (do sai số số thực float)
    int indices[9];
    
    // Kiểm tra lần lượt từng mặt trong 6 mặt
    for (int face = 0; face < 6; face++) {
        // Lấy danh sách 9 mảnh thuộc mặt này
        getFaceIndices(face, indices);
        
        // Lấy màu của mảnh trung tâm (chỉ số 4 trong mảng 9 phần tử) làm màu chuẩn
        // Mảnh trung tâm của một mặt không bao giờ thay đổi vị trí tương đối
        const float* centerColor = g_rubikCube.pieces[indices[4]].colors[face];
        
        // So sánh màu của 8 mảnh còn lại với mảnh trung tâm
        for (int i = 0; i < 9; i++) {
            const float* sticker = g_rubikCube.pieces[indices[i]].colors[face];
            
            // Tính khoảng cách màu (Manhattan distance) để so sánh
            float diff = fabs(sticker[0] - centerColor[0]) +
                        fabs(sticker[1] - centerColor[1]) +
                        fabs(sticker[2] - centerColor[2]);
            
            // Nếu sự khác biệt lớn hơn dung sai -> màu không khớp -> chưa giải
            if (diff > tolerance) {
                return false;
            }
        }
    }
    return true;  // Tất cả các mặt đều đồng màu
}

void testRotationIdentity() {
    if (g_logFile == NULL) {
        return;
    }
    
    fprintf(g_logFile, "\n=== KIỂM TRA TÍNH ĐỒNG NHẤT XOAỸ ===\n");
    
    float originalColors[27][6][3];
    for (int p = 0; p < 27; p++) {
        for (int f = 0; f < 6; f++) {
            for (int c = 0; c < 3; c++) {
                originalColors[p][f][c] = g_rubikCube.pieces[p].colors[f][c];
            }
        }
    }
    
    const Face facesToTest[] = {FRONT, BACK, LEFT, RIGHT, UP, DOWN};
    const char* faceNames[] = {"FRONT", "BACK", "LEFT", "RIGHT", "UP", "DOWN"};
    const int entriesPerCube = 27 * 6 * 3;
    
    for (int faceIdx = 0; faceIdx < 6; faceIdx++) {
        Face face = facesToTest[faceIdx];
        for (int p = 0; p < 27; p++) {
            for (int f = 0; f < 6; f++) {
                for (int c = 0; c < 3; c++) {
                    g_rubikCube.pieces[p].colors[f][c] = originalColors[p][f][c];
                }
            }
        }
        
        fprintf(g_logFile, "Kiểm tra %s: thực hiện 4 lượt xoay CW...\n", faceNames[faceIdx]);
        extern void rotateFace(int face, bool clockwise);
        for (int turn = 0; turn < 4; turn++) {
            rotateFace(face, true);
        }
        
        int matches = 0;
        for (int p = 0; p < 27; p++) {
            for (int f = 0; f < 6; f++) {
                for (int c = 0; c < 3; c++) {
                    float diff = fabs(g_rubikCube.pieces[p].colors[f][c] - originalColors[p][f][c]);
                    if (diff < 0.001f) {
                        matches++;
                    }
                }
            }
        }
        
        if (matches == entriesPerCube) {
            fprintf(g_logFile, "  -> %s THÀNH CÔNG (%d/%d khớp)\n", faceNames[faceIdx], matches, entriesPerCube);
        } else {
            fprintf(g_logFile, "  -> %s THẤT BẠI (%d/%d khớp)\n", faceNames[faceIdx], matches, entriesPerCube);
        }
    }
    
    for (int p = 0; p < 27; p++) {
        for (int f = 0; f < 6; f++) {
            for (int c = 0; c < 3; c++) {
                g_rubikCube.pieces[p].colors[f][c] = originalColors[p][f][c];
            }
        }
    }
    
    fprintf(g_logFile, "=== KẾT THÚC KIỂM TRA TÍNH ĐỒNG NHẤT XOAỸ ===\n\n");
    fflush(g_logFile);
}
