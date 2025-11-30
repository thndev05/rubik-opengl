#ifndef RUBIK_ANIMATION_H
#define RUBIK_ANIMATION_H

#include "rubik_types.h"

// Global animation state
extern RotationAnimation g_animation;
extern MoveQueue g_moveQueue;
extern int g_lastTimeMs;
extern bool g_keyHeld[256];
extern int g_scrambleMovesPending;

// Animation control
void startRotation(Face face, bool clockwise, bool isScrambleMove = false);
void updateAnimation(float deltaTime);
void cancelAnimationAndQueue();
bool isPieceInAnimation(int pieceIndex);
float easeInOutCubic(float t);

// Queue management
bool dequeueQueuedMove(Face& face, bool& clockwise, bool& isScrambleMove);

// Timer callback
void idle();

#endif // RUBIK_ANIMATION_H
