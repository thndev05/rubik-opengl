#ifndef RUBIK_TYPES_H
#define RUBIK_TYPES_H

// Face orientation enum
enum Face {
    FRONT = 0,  // Red
    BACK = 1,   // Orange
    LEFT = 2,   // Blue
    RIGHT = 3,  // Green
    UP = 4,     // White
    DOWN = 5    // Yellow
};

// CubePiece structure - represents one piece of the Rubik's Cube
struct CubePiece {
    int position[3];        // Grid position: x,y,z in {-1, 0, +1}
    float colors[6][3];    // 6 faces RGB: [0=Front,1=Back,2=Left,3=Right,4=Up,5=Down]
    bool isVisible;         // Whether this piece is visible
};

// RubikCube structure - contains all 27 pieces
struct RubikCube {
    CubePiece pieces[27];  // Fixed array 3x3x3 = 27 pieces
    float pieceSize;        // Size of each piece
    float gapSize;         // Gap between pieces
};

// Animation state
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

// Move queue
struct MoveQueue {
    int moves[20];           // Queue capacity
    bool dirs[20];
    bool scrambleFlags[20];
    int count;
    int head;
};

// Timer state
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

// View face mapping for dynamic rotation
struct ViewFaceMapping {
    Face front;
    Face back;
    Face left;
    Face right;
    Face up;
    Face down;
};

#endif // RUBIK_TYPES_H
