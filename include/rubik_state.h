#ifndef RUBIK_STATE_H
#define RUBIK_STATE_H

#include "rubik_types.h"
#include <cstdio>
#include <ctime>

// Global Rubik's Cube instance
extern RubikCube g_rubikCube;

// Debugging
extern FILE* g_logFile;
extern clock_t g_logStartClock;

// Initialize and manage cube state
void initRubikCube();
void resetCube();
void shuffleCube(int numMoves);
bool isCubeSolved();

// Utility functions
int positionToIndex(int i, int j, int k);
void getFaceIndices(int face, int indices[9]);
int encodePositionKey(int x, int y, int z);

// Logging functions
void initLogFile();
void closeLogFile();
double getLogTimestampMs();

// Test functions
void testRotationIdentity();

#endif // RUBIK_STATE_H
