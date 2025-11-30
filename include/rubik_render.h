#ifndef RUBIK_RENDER_H
#define RUBIK_RENDER_H

#include "rubik_types.h"

// OpenGL initialization
void initOpenGL();

// Drawing functions
void drawCubePiece(const CubePiece& piece);
void drawRubikCube();

// Display callbacks
void display();
void reshape(int w, int h);

// Rotation helpers
void rotateAroundAxis(const float axis[3], float angle);
float clampAngle(float angle, float minAngle, float maxAngle);

#endif // RUBIK_RENDER_H
