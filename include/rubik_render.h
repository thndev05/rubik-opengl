#ifndef RUBIK_RENDER_H
#define RUBIK_RENDER_H

#include "rubik_types.h"

// Khởi tạo OpenGL
void initOpenGL();

// Hàm vẽ
void drawCubePiece(const CubePiece& piece);
void drawRubikCube();

// Callback hiển thị
void display();
void reshape(int w, int h);

// Hỗ trợ xoay
void rotateAroundAxis(const float axis[3], float angle);
float clampAngle(float angle, float minAngle, float maxAngle);

#endif // RUBIK_RENDER_H
