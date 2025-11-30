#include "rubik_state.h"
#include "rubik_constants.h"
#include "rubik_animation.h"
#include "rubik_timer.h"
#include <cstdio>
#include <ctime>
#include <cstring>
#include <cmath>
#include <cstdlib>

RubikCube g_rubikCube;
FILE* g_logFile = NULL;
clock_t g_logStartClock = 0;

void initLogFile() {
    g_logFile = fopen("rubik_debug.log", "w");
    if (g_logFile == NULL) {
        return;
    }
    g_logStartClock = clock();
    
    time_t rawTime;
    struct tm* timeInfo;
    char timeStr[80];
    
    time(&rawTime);
    timeInfo = localtime(&rawTime);
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", timeInfo);
    
    fprintf(g_logFile, "=== Rubik's Cube Debug Log ===\n");
    fprintf(g_logFile, "Started: %s\n", timeStr);
    fprintf(g_logFile, "==============================\n\n");
    fflush(g_logFile);
}

double getLogTimestampMs() {
    if (g_logStartClock == 0) {
        return 0.0;
    }
    clock_t current = clock();
    return (double)(current - g_logStartClock) * 1000.0 / CLOCKS_PER_SEC;
}

void closeLogFile() {
    if (g_logFile != NULL) {
        fprintf(g_logFile, "\n=== Log End ===\n");
        fclose(g_logFile);
        g_logFile = NULL;
    }
}

int positionToIndex(int i, int j, int k) {
    int kOffset = (k == 1) ? 0 : (k == 0) ? 9 : 18;
    int jOffset = (j + 1) * 3;
    int iOffset = (i + 1);
    return kOffset + jOffset + iOffset;
}

void getFaceIndices(int face, int indices[9]) {
    int idx = 0;
    int i, j, k;
    
    switch (face) {
        case FRONT:
            k = 1;
            for (j = -1; j <= 1; j++) {
                for (i = -1; i <= 1; i++) {
                    indices[idx++] = positionToIndex(i, j, k);
                }
            }
            break;
            
        case BACK:
            k = -1;
            for (j = -1; j <= 1; j++) {
                for (i = -1; i <= 1; i++) {
                    indices[idx++] = positionToIndex(i, j, k);
                }
            }
            break;
            
        case LEFT:
            i = -1;
            for (j = -1; j <= 1; j++) {
                for (k = 1; k >= -1; k--) {
                    indices[idx++] = positionToIndex(i, j, k);
                }
            }
            break;
            
        case RIGHT:
            i = 1;
            for (j = -1; j <= 1; j++) {
                for (k = 1; k >= -1; k--) {
                    indices[idx++] = positionToIndex(i, j, k);
                }
            }
            break;
            
        case UP:
            j = 1;
            for (k = 1; k >= -1; k--) {
                for (i = -1; i <= 1; i++) {
                    indices[idx++] = positionToIndex(i, j, k);
                }
            }
            break;
            
        case DOWN:
            j = -1;
            for (k = 1; k >= -1; k--) {
                for (i = -1; i <= 1; i++) {
                    indices[idx++] = positionToIndex(i, j, k);
                }
            }
            break;
    }
}

int encodePositionKey(int x, int y, int z) {
    return (x + 1) * 9 + (y + 1) * 3 + (z + 1);
}

void initRubikCube() {
    g_rubikCube.pieceSize = PIECE_SIZE;
    g_rubikCube.gapSize = GAP_SIZE;
    
    int index = 0;
    for (int k = 1; k >= -1; k--) {
        for (int j = -1; j <= 1; j++) {
            for (int i = -1; i <= 1; i++) {
                CubePiece& piece = g_rubikCube.pieces[index];
                
                piece.position[0] = i;
                piece.position[1] = j;
                piece.position[2] = k;
                piece.isVisible = true;
                
                for (int face = 0; face < 6; face++) {
                    piece.colors[face][0] = COLOR_BLACK[0];
                    piece.colors[face][1] = COLOR_BLACK[1];
                    piece.colors[face][2] = COLOR_BLACK[2];
                }
                
                if (k == 1) {
                    piece.colors[0][0] = COLOR_RED[0];
                    piece.colors[0][1] = COLOR_RED[1];
                    piece.colors[0][2] = COLOR_RED[2];
                }
                
                if (k == -1) {
                    piece.colors[1][0] = COLOR_ORANGE[0];
                    piece.colors[1][1] = COLOR_ORANGE[1];
                    piece.colors[1][2] = COLOR_ORANGE[2];
                }
                
                if (i == -1) {
                    piece.colors[2][0] = COLOR_BLUE[0];
                    piece.colors[2][1] = COLOR_BLUE[1];
                    piece.colors[2][2] = COLOR_BLUE[2];
                }
                
                if (i == 1) {
                    piece.colors[3][0] = COLOR_GREEN[0];
                    piece.colors[3][1] = COLOR_GREEN[1];
                    piece.colors[3][2] = COLOR_GREEN[2];
                }
                
                if (j == 1) {
                    piece.colors[4][0] = COLOR_WHITE[0];
                    piece.colors[4][1] = COLOR_WHITE[1];
                    piece.colors[4][2] = COLOR_WHITE[2];
                }
                
                if (j == -1) {
                    piece.colors[5][0] = COLOR_YELLOW[0];
                    piece.colors[5][1] = COLOR_YELLOW[1];
                    piece.colors[5][2] = COLOR_YELLOW[2];
                }
                
                index++;
            }
        }
    }
    
    if (g_logFile != NULL) {
        fprintf(g_logFile, "Phase 2: Initialized %d Rubik pieces\n", 27);
        fflush(g_logFile);
    }
}

void resetCube() {
    cancelAnimationAndQueue();
    initRubikCube();
    extern int g_scrambleMovesPending;
    g_scrambleMovesPending = 0;
    resetTimerState();
    if (g_logFile != NULL) {
        fprintf(g_logFile, "RESET: Cube to solved state\n");
        fflush(g_logFile);
    }
}

void shuffleCube(int numMoves) {
    if (numMoves <= 0) {
        return;
    }
    resetTimerState();
    extern int g_scrambleMovesPending;
    g_scrambleMovesPending = numMoves;
    for (int i = 0; i < numMoves; i++) {
        Face face = static_cast<Face>(rand() % 6);
        bool clockwise = (rand() % 2) == 0;
        startRotation(face, clockwise, true);
    }
    if (g_logFile != NULL) {
        fprintf(g_logFile, "SHUFFLE: %d random moves queued\n", numMoves);
        fflush(g_logFile);
    }
}

bool isCubeSolved() {
    const float tolerance = 0.05f;
    int indices[9];
    for (int face = 0; face < 6; face++) {
        getFaceIndices(face, indices);
        const float* centerColor = g_rubikCube.pieces[indices[4]].colors[face];
        for (int i = 0; i < 9; i++) {
            const float* sticker = g_rubikCube.pieces[indices[i]].colors[face];
            float diff = fabs(sticker[0] - centerColor[0]) +
                        fabs(sticker[1] - centerColor[1]) +
                        fabs(sticker[2] - centerColor[2]);
            if (diff > tolerance) {
                return false;
            }
        }
    }
    return true;
}

void testRotationIdentity() {
    if (g_logFile == NULL) {
        return;
    }
    
    fprintf(g_logFile, "\n=== ROTATION IDENTITY TEST ===\n");
    
    float originalColors[27][6][3];
    for (int p = 0; p < 27; p++) {
        for (int f = 0; f < 6; f++) {
            for (int c = 0; c < 3; c++) {
                originalColors[p][f][c] = g_rubikCube.pieces[p].colors[f][c];
            }
        }
    }
    
    const Face facesToTest[] = {FRONT, BACK, LEFT, RIGHT, UP, DOWN};
    const char* faceNames[] = {"FRONT", "BACK", "LEFT", "RIGHT", "UP", "DOWN"};
    const int entriesPerCube = 27 * 6 * 3;
    
    for (int faceIdx = 0; faceIdx < 6; faceIdx++) {
        Face face = facesToTest[faceIdx];
        for (int p = 0; p < 27; p++) {
            for (int f = 0; f < 6; f++) {
                for (int c = 0; c < 3; c++) {
                    g_rubikCube.pieces[p].colors[f][c] = originalColors[p][f][c];
                }
            }
        }
        
        fprintf(g_logFile, "Testing %s: performing 4 CW turns...\n", faceNames[faceIdx]);
        extern void rotateFace(int face, bool clockwise);
        for (int turn = 0; turn < 4; turn++) {
            rotateFace(face, true);
        }
        
        int matches = 0;
        for (int p = 0; p < 27; p++) {
            for (int f = 0; f < 6; f++) {
                for (int c = 0; c < 3; c++) {
                    float diff = fabs(g_rubikCube.pieces[p].colors[f][c] - originalColors[p][f][c]);
                    if (diff < 0.001f) {
                        matches++;
                    }
                }
            }
        }
        
        if (matches == entriesPerCube) {
            fprintf(g_logFile, "  -> %s PASSED (%d/%d matches)\n", faceNames[faceIdx], matches, entriesPerCube);
        } else {
            fprintf(g_logFile, "  -> %s FAILED (%d/%d matches)\n", faceNames[faceIdx], matches, entriesPerCube);
        }
    }
    
    for (int p = 0; p < 27; p++) {
        for (int f = 0; f < 6; f++) {
            for (int c = 0; c < 3; c++) {
                g_rubikCube.pieces[p].colors[f][c] = originalColors[p][f][c];
            }
        }
    }
    
    fprintf(g_logFile, "=== END ROTATION IDENTITY TEST ===\n\n");
    fflush(g_logFile);
}
