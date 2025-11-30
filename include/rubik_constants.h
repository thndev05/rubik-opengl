#ifndef RUBIK_CONSTANTS_H
#define RUBIK_CONSTANTS_H

// Kích thước cửa sổ
extern int g_windowWidth;
extern int g_windowHeight;

// Hằng số Rubik's Cube
const float PIECE_SIZE = 0.9f;  // Kích thước mỗi mảnh cube
const float GAP_SIZE = 0.1f;    // Khoảng cách giữa các mảnh

// Màu sắc chuẩn của Rubik's Cube
// Chỉ số mặt: 0=Front, 1=Back, 2=Left, 3=Right, 4=Up, 5=Down
const float COLOR_WHITE[] = {1.0f, 1.0f, 1.0f};   // Trên (Up)
const float COLOR_YELLOW[] = {1.0f, 1.0f, 0.0f};  // Dưới (Down)
const float COLOR_RED[] = {1.0f, 0.0f, 0.0f};     // Trước (Front)
const float COLOR_ORANGE[] = {1.0f, 0.5f, 0.0f};  // Sau (Back)
const float COLOR_GREEN[] = {0.0f, 1.0f, 0.0f};   // Phải (Right)
const float COLOR_BLUE[] = {0.0f, 0.0f, 1.0f};    // Trái (Left)
const float COLOR_BLACK[] = {0.1f, 0.1f, 0.1f};   // Mặt ẩn

// Hằng số animation
const float ROTATION_SPEED_DEG_PER_SEC = 360.0f;
const int MOVE_QUEUE_CAPACITY = 20;

// Hằng số camera
const float ROTATION_SENSITIVITY = 0.3f;
const float KEYBOARD_ROTATION_SPEED = 5.0f;
const float CAMERA_DISTANCE = 8.0f;

#endif // RUBIK_CONSTANTS_H
