#ifndef RUBIK_STATE_H
#define RUBIK_STATE_H

#include "rubik_types.h"
#include <cstdio>
#include <ctime>

// Instance toàn cục Rubik's Cube
extern RubikCube g_rubikCube;

// Debug
extern FILE* g_logFile;
extern clock_t g_logStartClock;

// Khởi tạo và quản lý trạng thái cube
void initRubikCube();
void resetCube();
void shuffleCube(int numMoves);
bool isCubeSolved();

// Hàm tiện ích
int positionToIndex(int i, int j, int k);
void getFaceIndices(int face, int indices[9]);
int encodePositionKey(int x, int y, int z);

// Hàm logging
void initLogFile();
void closeLogFile();
double getLogTimestampMs();

// Hàm kiểm tra
void testRotationIdentity();

#endif // RUBIK_STATE_H
