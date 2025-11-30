#ifndef RUBIK_ROTATION_H
#define RUBIK_ROTATION_H

#include "rubik_types.h"

// Hàm xoay chính
void rotateFace(int face, bool clockwise);

// Xoay vị trí
void rotatePositions(int face, bool clockwise);

// Xoay hướng mảnh
void rotatePieceOrientation(int pieceIndex, int axis, bool clockwise);

// Xoay tọa độ
void rotateCoordinates(int axis, int axisSign, bool clockwise,
                       int x, int y, int z,
                       int& rx, int& ry, int& rz);

// Xoay vector
void rotateVectorAroundAxis(const float axis[3], float angleDegrees,
                            float& x, float& y, float& z);

// Thao tác ma trận
void axisAngleToMatrix(float m[3][3], const float axis[3], float angleDegrees);
void matrixMultiply(float result[3][3], const float a[3][3], const float b[3][3]);

// Tiện ích cho mặt
Face getOppositeFace(Face face);
Face getAbsoluteFace(int relativeFace);

#endif // RUBIK_ROTATION_H
