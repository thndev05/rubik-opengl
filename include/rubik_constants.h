#ifndef RUBIK_CONSTANTS_H
#define RUBIK_CONSTANTS_H

// Window dimensions
extern int g_windowWidth;
extern int g_windowHeight;

// Rubik's Cube constants
const float PIECE_SIZE = 0.9f;  // Size of each cube piece
const float GAP_SIZE = 0.1f;    // Gap between pieces

// Standard Rubik's Cube colors
// Face indices: 0=Front, 1=Back, 2=Left, 3=Right, 4=Up, 5=Down
const float COLOR_WHITE[] = {1.0f, 1.0f, 1.0f};   // Up
const float COLOR_YELLOW[] = {1.0f, 1.0f, 0.0f};  // Down
const float COLOR_RED[] = {1.0f, 0.0f, 0.0f};     // Front
const float COLOR_ORANGE[] = {1.0f, 0.5f, 0.0f};  // Back
const float COLOR_GREEN[] = {0.0f, 1.0f, 0.0f};   // Right
const float COLOR_BLUE[] = {0.0f, 0.0f, 1.0f};    // Left
const float COLOR_BLACK[] = {0.1f, 0.1f, 0.1f};   // Hidden faces

// Animation constants
const float ROTATION_SPEED_DEG_PER_SEC = 360.0f;
const int MOVE_QUEUE_CAPACITY = 20;

// Camera constants
const float ROTATION_SENSITIVITY = 0.3f;
const float KEYBOARD_ROTATION_SPEED = 5.0f;
const float CAMERA_DISTANCE = 8.0f;

#endif // RUBIK_CONSTANTS_H
