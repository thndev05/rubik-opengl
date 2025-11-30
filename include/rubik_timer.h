#ifndef RUBIK_TIMER_H
#define RUBIK_TIMER_H

#include "rubik_types.h"

// Global timer state
extern SpeedTimer g_timer;

// Timer functions
void resetTimerState();
void armTimerForSolve();
void updateTimer();
void onMoveStarted();
void handleScrambleMoveCompletion(bool wasScrambleMove);

// Timer display
void displayTimerOverlay();
void renderBitmapString(float x, float y, void* font, const char* string);
void formatTimerText(float seconds, char* buffer, int bufferSize);

#endif // RUBIK_TIMER_H
