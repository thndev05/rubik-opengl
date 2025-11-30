#include "rubik_timer.h"
#include "rubik_state.h"
#include "rubik_animation.h"
#include "rubik_constants.h"
#include <GL/glut.h>
#include <cstdio>

#if defined(_MSC_VER) && !defined(snprintf)
#define snprintf _snprintf
#endif

SpeedTimer g_timer = {TIMER_IDLE, 0.0f, 0.0f, 0, 0.0f, 0.0f, 0.0f};

void resetTimerState() {
    g_timer.state = TIMER_IDLE;
    g_timer.startTime = 0.0f;
    g_timer.endTime = 0.0f;
    g_timer.moveCount = 0;
    g_timer.currentTime = 0.0f;
    g_timer.tps = 0.0f;
    g_timer.lastSampleTime = 0.0f;
}

void armTimerForSolve() {
    g_timer.state = TIMER_READY;
    g_timer.startTime = 0.0f;
    g_timer.endTime = 0.0f;
    g_timer.moveCount = 0;
    g_timer.currentTime = 0.0f;
    g_timer.tps = 0.0f;
    g_timer.lastSampleTime = 0.0f;
}

void handleScrambleMoveCompletion(bool wasScrambleMove) {
    if (!wasScrambleMove || g_scrambleMovesPending <= 0) {
        return;
    }
    g_scrambleMovesPending--;
    if (g_scrambleMovesPending == 0) {
        armTimerForSolve();
    }
}

void onMoveStarted() {
    if (g_timer.state == TIMER_READY) {
        g_timer.state = TIMER_RUNNING;
        g_timer.startTime = (float)glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
        g_timer.lastSampleTime = g_timer.startTime;
        g_timer.moveCount = 0;
        g_timer.currentTime = 0.0f;
        g_timer.tps = 0.0f;
        if (g_logFile != NULL) {
            fprintf(g_logFile, "TIMER ĐÃ BẮT ĐẦU\n");
            fflush(g_logFile);
        }
    }
    if (g_timer.state == TIMER_RUNNING) {
        g_timer.moveCount++;
    }
}

void updateTimer() {
    if (g_timer.state != TIMER_RUNNING) {
        return;
    }
    float now = (float)glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    g_timer.currentTime = now - g_timer.startTime;
    g_timer.lastSampleTime = now;
    if (g_timer.currentTime < 0.0f) {
        g_timer.currentTime = 0.0f;
    }
    if (g_timer.currentTime > 0.0f) {
        g_timer.tps = (float)g_timer.moveCount / g_timer.currentTime;
    }
    if (!g_animation.isActive && isCubeSolved()) {
        g_timer.state = TIMER_STOPPED;
        g_timer.endTime = g_timer.currentTime;
        if (g_logFile != NULL) {
            fprintf(g_logFile, "========================================\n");
            fprintf(g_logFile, "ĐÃ GIẢI XONG CUBE!\n");
            fprintf(g_logFile, "Thời gian: %.2f giây\n", g_timer.endTime);
            fprintf(g_logFile, "Số nước: %d\n", g_timer.moveCount);
            fprintf(g_logFile, "TPS: %.2f\n", g_timer.tps);
            fprintf(g_logFile, "========================================\n");
            fflush(g_logFile);
        }
    }
    glutPostRedisplay();
}

void renderBitmapString(float x, float y, void* font, const char* string) {
    glRasterPos2f(x, y);
    while (*string != '\0') {
        glutBitmapCharacter(font, *string);
        ++string;
    }
}

void formatTimerText(float seconds, char* buffer, int bufferSize) {
    if (seconds < 0.0f) {
        seconds = 0.0f;
    }
    int minutes = (int)(seconds / 60.0f);
    float sec = seconds - minutes * 60.0f;
    if (bufferSize > 0) {
        snprintf(buffer, bufferSize, "%02d:%06.3f", minutes, sec);
    }
}

void displayTimerOverlay() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0.0f, (float)g_windowWidth, 0.0f, (float)g_windowHeight);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    GLboolean depthEnabled = glIsEnabled(GL_DEPTH_TEST);
    GLboolean lightingEnabled = glIsEnabled(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    char buffer[128];
    if (g_scrambleMovesPending > 0) {
        glColor3f(1.0f, 0.6f, 0.0f);
        snprintf(buffer, sizeof(buffer), "Scrambling... (%d moves left)", g_scrambleMovesPending);
        renderBitmapString(10.0f, g_windowHeight - 20.0f, GLUT_BITMAP_HELVETICA_18, buffer);
        renderBitmapString(10.0f, g_windowHeight - 40.0f, GLUT_BITMAP_HELVETICA_18,
                           "Please wait for scramble to finish");
    } else {
        switch (g_timer.state) {
            case TIMER_IDLE:
                glColor3f(1.0f, 1.0f, 1.0f);
                renderBitmapString(10.0f, g_windowHeight - 20.0f, GLUT_BITMAP_HELVETICA_18,
                                   "Press 'S' to scramble");
                break;
            case TIMER_READY:
                glColor3f(1.0f, 1.0f, 0.2f);
                renderBitmapString(10.0f, g_windowHeight - 20.0f, GLUT_BITMAP_HELVETICA_18,
                                   "READY - Make a move to start");
                break;
            case TIMER_RUNNING:
                glColor3f(0.0f, 1.0f, 0.0f);
                formatTimerText(g_timer.currentTime, buffer, sizeof(buffer));
                renderBitmapString(10.0f, g_windowHeight - 20.0f, GLUT_BITMAP_HELVETICA_18,
                                   buffer);
                snprintf(buffer, sizeof(buffer), "Moves: %d", g_timer.moveCount);
                renderBitmapString(10.0f, g_windowHeight - 40.0f, GLUT_BITMAP_HELVETICA_18,
                                   buffer);
                snprintf(buffer, sizeof(buffer), "TPS: %.2f", g_timer.tps);
                renderBitmapString(10.0f, g_windowHeight - 60.0f, GLUT_BITMAP_HELVETICA_18,
                                   buffer);
                break;
            case TIMER_STOPPED:
                glColor3f(0.2f, 1.0f, 0.2f);
                snprintf(buffer, sizeof(buffer), "Solved! Time %.2fs | Moves %d | TPS %.2f",
                         g_timer.endTime, g_timer.moveCount, g_timer.tps);
                renderBitmapString(g_windowWidth * 0.2f, g_windowHeight * 0.5f,
                                   GLUT_BITMAP_HELVETICA_18, buffer);
                break;
        }
    }
    if (depthEnabled) {
        glEnable(GL_DEPTH_TEST);
    }
    if (lightingEnabled) {
        glEnable(GL_LIGHTING);
    }
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}
