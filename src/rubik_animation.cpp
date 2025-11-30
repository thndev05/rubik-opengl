#include "rubik_animation.h"
#include "rubik_state.h"
#include "rubik_rotation.h"
#include "rubik_timer.h"
#include "rubik_constants.h"
#include <GL/glut.h>
#include <cstdio>

RotationAnimation g_animation = {
    false,
    FRONT,
    true,
    false,
    0.0f,
    90.0f,
    ROTATION_SPEED_DEG_PER_SEC,
    0.0f,
    {-1, -1, -1, -1, -1, -1, -1, -1, -1}
};

MoveQueue g_moveQueue = {{0}, {false}, {false}, 0, 0};
int g_lastTimeMs = 0;
bool g_keyHeld[256] = {false};
int g_scrambleMovesPending = 0;

float easeInOutCubic(float t) {
    if (t < 0.0f) {
        t = 0.0f;
    } else if (t > 1.0f) {
        t = 1.0f;
    }
    if (t < 0.5f) {
        return 4.0f * t * t * t;
    }
    float f = (2.0f * t) - 2.0f;
    return 0.5f * f * f * f + 1.0f;
}

bool isPieceInAnimation(int pieceIndex) {
    if (!g_animation.isActive) {
        return false;
    }
    for (int i = 0; i < 9; i++) {
        if (g_animation.affectedIndices[i] == pieceIndex) {
            return true;
        }
    }
    return false;
}

void cancelAnimationAndQueue() {
    g_animation.isActive = false;
    g_animation.isScrambleMove = false;
    g_animation.currentAngle = 0.0f;
    g_animation.displayAngle = 0.0f;
    for (int i = 0; i < 9; i++) {
        g_animation.affectedIndices[i] = -1;
    }
    g_moveQueue.count = 0;
    g_moveQueue.head = 0;
    for (int i = 0; i < MOVE_QUEUE_CAPACITY; i++) {
        g_moveQueue.scrambleFlags[i] = false;
    }
}

bool dequeueQueuedMove(Face& face, bool& clockwise, bool& isScrambleMove) {
    if (g_moveQueue.count == 0) {
        return false;
    }
    int idx = g_moveQueue.head;
    face = static_cast<Face>(g_moveQueue.moves[idx]);
    clockwise = g_moveQueue.dirs[idx];
    isScrambleMove = g_moveQueue.scrambleFlags[idx];
    g_moveQueue.scrambleFlags[idx] = false;
    g_moveQueue.head = (g_moveQueue.head + 1) % MOVE_QUEUE_CAPACITY;
    g_moveQueue.count--;
    if (g_moveQueue.count == 0) {
        g_moveQueue.head = 0;
    }
    return true;
}

void startRotation(Face face, bool clockwise, bool isScrambleMove) {
    if (face < FRONT || face > DOWN) {
        return;
    }
    if (g_animation.isActive) {
        if (g_moveQueue.count >= MOVE_QUEUE_CAPACITY) {
            if (g_logFile != NULL) {
                const char* faceNames[] = {"FRONT", "BACK", "LEFT", "RIGHT", "UP", "DOWN"};
                double tsMs = getLogTimestampMs();
                fprintf(g_logFile, "[%010.3f ms] QUEUE FULL: drop %s %s\n",
                        tsMs,
                        faceNames[face],
                        clockwise ? "CW" : "CCW");
                fflush(g_logFile);
            }
        } else {
            int idx = (g_moveQueue.head + g_moveQueue.count) % MOVE_QUEUE_CAPACITY;
            g_moveQueue.moves[idx] = static_cast<int>(face);
            g_moveQueue.dirs[idx] = clockwise;
            g_moveQueue.scrambleFlags[idx] = isScrambleMove;
            g_moveQueue.count++;
            if (g_logFile != NULL) {
                const char* faceNames[] = {"FRONT", "BACK", "LEFT", "RIGHT", "UP", "DOWN"};
                double tsMs = getLogTimestampMs();
                fprintf(g_logFile, "[%010.3f ms] ANIM QUEUED %s %s | queue=%d\n",
                        tsMs,
                        faceNames[face],
                        clockwise ? "CW" : "CCW",
                        g_moveQueue.count);
                fflush(g_logFile);
            }
        }
        return;
    }
    onMoveStarted();
    g_animation.isActive = true;
    g_animation.face = face;
    g_animation.clockwise = clockwise;
    g_animation.isScrambleMove = isScrambleMove;
    g_animation.currentAngle = 0.0f;
    g_animation.displayAngle = 0.0f;
    g_animation.targetAngle = 90.0f;
    g_animation.speed = ROTATION_SPEED_DEG_PER_SEC;
    getFaceIndices(face, g_animation.affectedIndices);
    if (g_logFile != NULL) {
        const char* faceNames[] = {"FRONT", "BACK", "LEFT", "RIGHT", "UP", "DOWN"};
        double tsMs = getLogTimestampMs();
        fprintf(g_logFile, "[%010.3f ms] ANIM START %s %s | queue=%d\n",
                tsMs,
                faceNames[face],
                clockwise ? "CW" : "CCW",
                g_moveQueue.count);
        fflush(g_logFile);
    }
    glutPostRedisplay();
}

void updateAnimation(float deltaTime) {
    if (!g_animation.isActive) {
        return;
    }
    g_animation.currentAngle += g_animation.speed * deltaTime;
    if (g_animation.currentAngle > g_animation.targetAngle) {
        g_animation.currentAngle = g_animation.targetAngle;
    }
    float progress = (g_animation.targetAngle > 0.0f)
        ? (g_animation.currentAngle / g_animation.targetAngle)
        : 1.0f;
    if (progress > 1.0f) {
        progress = 1.0f;
    }
    g_animation.displayAngle = easeInOutCubic(progress) * g_animation.targetAngle;
    if (g_animation.currentAngle >= g_animation.targetAngle - 0.0001f) {
        Face finishedFace = g_animation.face;
        bool finishedDir = g_animation.clockwise;
        bool finishedWasScramble = g_animation.isScrambleMove;
        rotateFace(finishedFace, finishedDir);
        g_animation.isActive = false;
        g_animation.isScrambleMove = false;
        g_animation.currentAngle = 0.0f;
        g_animation.displayAngle = 0.0f;
        for (int i = 0; i < 9; i++) {
            g_animation.affectedIndices[i] = -1;
        }
        if (g_logFile != NULL) {
            const char* faceNames[] = {"FRONT", "BACK", "LEFT", "RIGHT", "UP", "DOWN"};
            double tsMs = getLogTimestampMs();
            fprintf(g_logFile, "[%010.3f ms] ANIM END %s %s | queue=%d\n",
                    tsMs,
                    faceNames[finishedFace],
                    finishedDir ? "CW" : "CCW",
                    g_moveQueue.count);
            fflush(g_logFile);
        }
        handleScrambleMoveCompletion(finishedWasScramble);
        Face nextFace;
        bool nextDir;
        bool nextIsScramble = false;
        if (dequeueQueuedMove(nextFace, nextDir, nextIsScramble)) {
            startRotation(nextFace, nextDir, nextIsScramble);
        }
    }
    glutPostRedisplay();
}

void idle() {
    int currentTime = glutGet(GLUT_ELAPSED_TIME);
    if (g_lastTimeMs == 0) {
        g_lastTimeMs = currentTime;
    }
    float deltaTime = (float)(currentTime - g_lastTimeMs) / 1000.0f;
    if (deltaTime > 0.1f) {
        deltaTime = 0.1f;
    } else if (deltaTime < 0.0f) {
        deltaTime = 0.0f;
    }
    g_lastTimeMs = currentTime;
    updateAnimation(deltaTime);
    updateTimer();
}
