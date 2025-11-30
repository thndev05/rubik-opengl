#ifndef RUBIK_TIMER_H
#define RUBIK_TIMER_H

#include "rubik_types.h"

// Trạng thái timer toàn cục
extern SpeedTimer g_timer;

// Hàm timer
void resetTimerState();
void armTimerForSolve();
void updateTimer();
void onMoveStarted();
void handleScrambleMoveCompletion(bool wasScrambleMove);

// Hiển thị timer
void displayTimerOverlay();
void renderBitmapString(float x, float y, void* font, const char* string);
void formatTimerText(float seconds, char* buffer, int bufferSize);

#endif // RUBIK_TIMER_H
