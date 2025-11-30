#include "rubik_rotation.h"
#include "rubik_state.h"
#include "rubik_constants.h"
#include "rubik_input.h"
#include <cmath>
#include <cstring>
#include <cstdio>

Face getOppositeFace(Face face) {
    switch (face) {
        case FRONT: return BACK;
        case BACK: return FRONT;
        case LEFT: return RIGHT;
        case RIGHT: return LEFT;
        case UP: return DOWN;
        case DOWN: return UP;
        default: return FRONT;
    }
}

void axisAngleToMatrix(float m[3][3], const float axis[3], float angleDegrees) {
    float angleRad = angleDegrees * 3.14159f / 180.0f;
    float c = cos(angleRad);
    float s = sin(angleRad);
    float t = 1.0f - c;
    
    float length = sqrt(axis[0] * axis[0] + axis[1] * axis[1] + axis[2] * axis[2]);
    float x, y, z;
    if (length > 0.0001f) {
        x = axis[0] / length;
        y = axis[1] / length;
        z = axis[2] / length;
    } else {
        x = axis[0];
        y = axis[1];
        z = axis[2];
    }
    
    m[0][0] = t * x * x + c;
    m[0][1] = t * x * y - s * z;
    m[0][2] = t * x * z + s * y;
    
    m[1][0] = t * x * y + s * z;
    m[1][1] = t * y * y + c;
    m[1][2] = t * y * z - s * x;
    
    m[2][0] = t * x * z - s * y;
    m[2][1] = t * y * z + s * x;
    m[2][2] = t * z * z + c;
}

void matrixMultiply(float result[3][3], const float a[3][3], const float b[3][3]) {
    int i, j, k;
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            result[i][j] = 0.0f;
            for (k = 0; k < 3; k++) {
                result[i][j] += a[i][k] * b[k][j];
            }
        }
    }
}

void rotateVectorAroundAxis(const float axis[3], float angleDegrees,
                            float& x, float& y, float& z) {
    if (fabs(angleDegrees) < 0.0001f) {
        return;
    }
    float rot[3][3];
    axisAngleToMatrix(rot, axis, angleDegrees);
    float rx = rot[0][0] * x + rot[0][1] * y + rot[0][2] * z;
    float ry = rot[1][0] * x + rot[1][1] * y + rot[1][2] * z;
    float rz = rot[2][0] * x + rot[2][1] * y + rot[2][2] * z;
    x = rx;
    y = ry;
    z = rz;
}

void rotateCoordinates(int axis, int axisSign, bool clockwise,
                       int x, int y, int z,
                       int& rx, int& ry, int& rz) {
    int angleSign = clockwise ? -1 : 1;
    angleSign *= axisSign;
    switch (axis) {
        case 0: // X-axis
            rx = x;
            if (angleSign > 0) {
                ry = -z;
                rz = y;
            } else {
                ry = z;
                rz = -y;
            }
            break;
        case 1: // Y-axis
            ry = y;
            if (angleSign > 0) {
                rz = -x;
                rx = z;
            } else {
                rz = x;
                rx = -z;
            }
            break;
        case 2: // Z-axis
            rz = z;
            if (angleSign > 0) {
                rx = -y;
                ry = x;
            } else {
                rx = y;
                ry = -x;
            }
            break;
        default:
            rx = x;
            ry = y;
            rz = z;
            break;
    }
}

void rotatePieceOrientation(int pieceIndex, int axis, bool clockwise) {
    CubePiece* p = &g_rubikCube.pieces[pieceIndex];
    float temp[3];
    int i;
    
    if (axis == 2) {  // Z-axis
        if (clockwise) {
            for (i = 0; i < 3; i++) temp[i] = p->colors[4][i];
            for (i = 0; i < 3; i++) p->colors[4][i] = p->colors[2][i];
            for (i = 0; i < 3; i++) p->colors[2][i] = p->colors[5][i];
            for (i = 0; i < 3; i++) p->colors[5][i] = p->colors[3][i];
            for (i = 0; i < 3; i++) p->colors[3][i] = temp[i];
        } else {
            for (i = 0; i < 3; i++) temp[i] = p->colors[4][i];
            for (i = 0; i < 3; i++) p->colors[4][i] = p->colors[3][i];
            for (i = 0; i < 3; i++) p->colors[3][i] = p->colors[5][i];
            for (i = 0; i < 3; i++) p->colors[5][i] = p->colors[2][i];
            for (i = 0; i < 3; i++) p->colors[2][i] = temp[i];
        }
    } else if (axis == 0) {  // X-axis
        if (clockwise) {
            for (i = 0; i < 3; i++) temp[i] = p->colors[0][i];
            for (i = 0; i < 3; i++) p->colors[0][i] = p->colors[5][i];
            for (i = 0; i < 3; i++) p->colors[5][i] = p->colors[1][i];
            for (i = 0; i < 3; i++) p->colors[1][i] = p->colors[4][i];
            for (i = 0; i < 3; i++) p->colors[4][i] = temp[i];
        } else {
            for (i = 0; i < 3; i++) temp[i] = p->colors[0][i];
            for (i = 0; i < 3; i++) p->colors[0][i] = p->colors[4][i];
            for (i = 0; i < 3; i++) p->colors[4][i] = p->colors[1][i];
            for (i = 0; i < 3; i++) p->colors[1][i] = p->colors[5][i];
            for (i = 0; i < 3; i++) p->colors[5][i] = temp[i];
        }
    } else if (axis == 1) {  // Y-axis
        if (clockwise) {
            for (i = 0; i < 3; i++) temp[i] = p->colors[0][i];
            for (i = 0; i < 3; i++) p->colors[0][i] = p->colors[2][i];
            for (i = 0; i < 3; i++) p->colors[2][i] = p->colors[1][i];
            for (i = 0; i < 3; i++) p->colors[1][i] = p->colors[3][i];
            for (i = 0; i < 3; i++) p->colors[3][i] = temp[i];
        } else {
            for (i = 0; i < 3; i++) temp[i] = p->colors[0][i];
            for (i = 0; i < 3; i++) p->colors[0][i] = p->colors[3][i];
            for (i = 0; i < 3; i++) p->colors[3][i] = p->colors[1][i];
            for (i = 0; i < 3; i++) p->colors[1][i] = p->colors[2][i];
            for (i = 0; i < 3; i++) p->colors[2][i] = temp[i];
        }
    }
}

void rotatePositions(int face, bool clockwise) {
    int indices[9];
    getFaceIndices(face, indices);
    
    float backupColors[9][6][3];
    int i, f, c;
    for (i = 0; i < 9; i++) {
        for (f = 0; f < 6; f++) {
            for (c = 0; c < 3; c++) {
                backupColors[i][f][c] = g_rubikCube.pieces[indices[i]].colors[f][c];
            }
        }
    }
    
    int rotationAxis;
    int axisSign = 1;
    switch (face) {
        case FRONT:
        case BACK:
            rotationAxis = 2;
            axisSign = (face == FRONT) ? 1 : -1;
            break;
        case LEFT:
        case RIGHT:
            rotationAxis = 0;
            axisSign = (face == RIGHT) ? 1 : -1;
            break;
        case UP:
        case DOWN:
            rotationAxis = 1;
            axisSign = (face == UP) ? 1 : -1;
            break;
        default:
            rotationAxis = 2;
            break;
    }
    
    int mapping[9];
    int keyToSlot[27];
    for (i = 0; i < 27; i++) {
        keyToSlot[i] = -1;
    }
    for (i = 0; i < 9; i++) {
        const CubePiece& piece = g_rubikCube.pieces[indices[i]];
        int key = encodePositionKey(piece.position[0], piece.position[1], piece.position[2]);
        keyToSlot[key] = i;
    }
    for (i = 0; i < 9; i++) {
        const CubePiece& piece = g_rubikCube.pieces[indices[i]];
        int rx, ry, rz;
        rotateCoordinates(rotationAxis, axisSign, clockwise,
                          piece.position[0], piece.position[1], piece.position[2],
                          rx, ry, rz);
        int key = encodePositionKey(rx, ry, rz);
        int destSlot = keyToSlot[key];
        mapping[destSlot] = i;
    }
    
    int j;
    int srcIdx;
    float temp[3];
    
    for (i = 0; i < 9; i++) {
        srcIdx = mapping[i];
        
        for (f = 0; f < 6; f++) {
            for (c = 0; c < 3; c++) {
                g_rubikCube.pieces[indices[i]].colors[f][c] = backupColors[srcIdx][f][c];
            }
        }
        
        if (i != 4) {
            CubePiece* p = &g_rubikCube.pieces[indices[i]];
            bool orientationClockwise = clockwise;
            if (axisSign < 0) {
                orientationClockwise = !orientationClockwise;
            }
            
            if (orientationClockwise) {
                if (rotationAxis == 2) {
                    for (j = 0; j < 3; j++) temp[j] = p->colors[4][j];
                    for (j = 0; j < 3; j++) p->colors[4][j] = p->colors[2][j];
                    for (j = 0; j < 3; j++) p->colors[2][j] = p->colors[5][j];
                    for (j = 0; j < 3; j++) p->colors[5][j] = p->colors[3][j];
                    for (j = 0; j < 3; j++) p->colors[3][j] = temp[j];
                } else if (rotationAxis == 0) {
                    for (j = 0; j < 3; j++) temp[j] = p->colors[0][j];
                    for (j = 0; j < 3; j++) p->colors[0][j] = p->colors[5][j];
                    for (j = 0; j < 3; j++) p->colors[5][j] = p->colors[1][j];
                    for (j = 0; j < 3; j++) p->colors[1][j] = p->colors[4][j];
                    for (j = 0; j < 3; j++) p->colors[4][j] = temp[j];
                } else if (rotationAxis == 1) {
                    for (j = 0; j < 3; j++) temp[j] = p->colors[0][j];
                    for (j = 0; j < 3; j++) p->colors[0][j] = p->colors[3][j];
                    for (j = 0; j < 3; j++) p->colors[3][j] = p->colors[1][j];
                    for (j = 0; j < 3; j++) p->colors[1][j] = p->colors[2][j];
                    for (j = 0; j < 3; j++) p->colors[2][j] = temp[j];
                }
            } else {
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
    
    if (g_logFile != NULL) {
        const char* faceNames[] = {"FRONT", "BACK", "LEFT", "RIGHT", "UP", "DOWN"};
        fprintf(g_logFile, "ROTATE %s %s: colors swapped, positions preserved\n",
                faceNames[face], clockwise ? "CW" : "CCW");
        fflush(g_logFile);
    }
}

void rotateFace(int face, bool clockwise) {
    int indices[9];
    getFaceIndices(face, indices);
    rotatePositions(face, clockwise);
    
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

Face getAbsoluteFace(int relativeFace) {
    ViewFaceMapping mapping;
    computeViewFaceMapping(mapping);
    Face selected = mapping.front;
    switch (relativeFace) {
        case 0: selected = mapping.front; break;
        case 1: selected = mapping.up; break;
        case 2: selected = mapping.right; break;
        case 3: selected = mapping.left; break;
        case 4: selected = mapping.down; break;
        case 5: selected = mapping.back; break;
        default: selected = mapping.front; break;
    }
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
