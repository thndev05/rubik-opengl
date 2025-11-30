#ifndef RUBIK_ROTATION_H
#define RUBIK_ROTATION_H

#include "rubik_types.h"

// Main rotation function
void rotateFace(int face, bool clockwise);

// Position rotation
void rotatePositions(int face, bool clockwise);

// Piece orientation rotation
void rotatePieceOrientation(int pieceIndex, int axis, bool clockwise);

// Coordinate rotation
void rotateCoordinates(int axis, int axisSign, bool clockwise,
                       int x, int y, int z,
                       int& rx, int& ry, int& rz);

// Vector rotation
void rotateVectorAroundAxis(const float axis[3], float angleDegrees,
                            float& x, float& y, float& z);

// Matrix operations
void axisAngleToMatrix(float m[3][3], const float axis[3], float angleDegrees);
void matrixMultiply(float result[3][3], const float a[3][3], const float b[3][3]);

// Face utilities
Face getOppositeFace(Face face);
Face getAbsoluteFace(int relativeFace);

#endif // RUBIK_ROTATION_H
