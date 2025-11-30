#ifndef RUBIK_TYPES_H
#define RUBIK_TYPES_H

// Enum hướng mặt
enum Face {
    FRONT = 0,  // Đỏ
    BACK = 1,   // Cam
    LEFT = 2,   // Xanh lá
    RIGHT = 3,  // Xanh dương
    UP = 4,     // Trắng
    DOWN = 5    // Vàng
};

// Cấu trúc CubePiece - đại diện cho một mảnh của Rubik's Cube
struct CubePiece {
    int position[3];        // Vị trí lưới: x,y,z trong {-1, 0, +1}
    float colors[6][3];    // 6 mặt RGB: [0=Front,1=Back,2=Left,3=Right,4=Up,5=Down]
    bool isVisible;         // Mảnh này có hiển thị không
};

// Cấu trúc RubikCube - chứa tất cả 27 mảnh
struct RubikCube {
    CubePiece pieces[27];  // Mảng cố định 3x3x3 = 27 mảnh
    float pieceSize;        // Kích thước mỗi mảnh
    float gapSize;         // Khoảng cách giữa các mảnh
};

// Trạng thái animation
struct RotationAnimation {
    bool isActive;
    Face face;
    bool clockwise;
    bool isScrambleMove;
    float currentAngle;
    float targetAngle;
    float speed;
    float displayAngle;
    int affectedIndices[9];
};

// Hàng đợi di chuyển
struct MoveQueue {
    int moves[20];           // Sức chứa hàng đợi
    bool dirs[20];
    bool scrambleFlags[20];
    int count;
    int head;
};

// Trạng thái timer
enum TimerState {
    TIMER_IDLE = 0,
    TIMER_READY,
    TIMER_RUNNING,
    TIMER_STOPPED
};

struct SpeedTimer {
    TimerState state;
    float startTime;
    float endTime;
    int moveCount;
    float currentTime;
    float tps;
    float lastSampleTime;
};

// Ánh xạ mặt theo góc nhìn cho xoay động
struct ViewFaceMapping {
    Face front;
    Face back;
    Face left;
    Face right;
    Face up;
    Face down;
};

#endif // RUBIK_TYPES_H
