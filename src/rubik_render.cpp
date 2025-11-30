#include "rubik_render.h"
#include "rubik_state.h"
#include "rubik_animation.h"
#include "rubik_input.h"
#include "rubik_timer.h"
#include "rubik_constants.h"
#include <GL/glut.h>
#include <cmath>

int g_windowWidth = 800;
int g_windowHeight = 600;

float clampAngle(float angle, float minAngle, float maxAngle) {
    if (angle < minAngle) {
        return minAngle;
    }
    if (angle > maxAngle) {
        return maxAngle;
    }
    return angle;
}

void rotateAroundAxis(const float axis[3], float angle) {
    float length = sqrt(axis[0] * axis[0] + axis[1] * axis[1] + axis[2] * axis[2]);
    if (length > 0.0001f) {
        float nx = axis[0] / length;
        float ny = axis[1] / length;
        float nz = axis[2] / length;
        glRotatef(angle, nx, ny, nz);
    }
}

void initOpenGL() {
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

#ifdef GL_MULTISAMPLE
    glEnable(GL_MULTISAMPLE);
#endif
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLineWidth(1.5f);
    
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glShadeModel(GL_SMOOTH);
    
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
    glDisable(GL_COLOR_MATERIAL);
}

void drawCubePiece(const CubePiece& piece) {
    const float size = g_rubikCube.pieceSize * 0.5f;
    
    glBegin(GL_QUADS);
    
    // Front face (Z+)
    glColor3fv(piece.colors[0]);
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-size, -size, size);
    glVertex3f(size, -size, size);
    glVertex3f(size, size, size);
    glVertex3f(-size, size, size);
    
    // Back face (Z-)
    glColor3fv(piece.colors[1]);
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(size, -size, -size);
    glVertex3f(-size, -size, -size);
    glVertex3f(-size, size, -size);
    glVertex3f(size, size, -size);
    
    // Left face (X-)
    glColor3fv(piece.colors[2]);
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(-size, -size, -size);
    glVertex3f(-size, -size, size);
    glVertex3f(-size, size, size);
    glVertex3f(-size, size, -size);
    
    // Right face (X+)
    glColor3fv(piece.colors[3]);
    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f(size, -size, size);
    glVertex3f(size, -size, -size);
    glVertex3f(size, size, -size);
    glVertex3f(size, size, size);
    
    // Up face (Y+)
    glColor3fv(piece.colors[4]);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-size, size, size);
    glVertex3f(size, size, size);
    glVertex3f(size, size, -size);
    glVertex3f(-size, size, -size);
    
    // Down face (Y-)
    glColor3fv(piece.colors[5]);
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(-size, -size, -size);
    glVertex3f(size, -size, -size);
    glVertex3f(size, -size, size);
    glVertex3f(-size, -size, size);
    
    glEnd();
}

void drawRubikCube() {
    glPushMatrix();
    
    for (int i = 0; i < 27; i++) {
        const CubePiece& piece = g_rubikCube.pieces[i];
        
        if (!piece.isVisible) {
            continue;
        }
        
        float worldX = (float)piece.position[0] * (g_rubikCube.pieceSize + g_rubikCube.gapSize);
        float worldY = (float)piece.position[1] * (g_rubikCube.pieceSize + g_rubikCube.gapSize);
        float worldZ = (float)piece.position[2] * (g_rubikCube.pieceSize + g_rubikCube.gapSize);
        
        glPushMatrix();
        
        bool pieceAnimating = g_animation.isActive && isPieceInAnimation(i);
        if (pieceAnimating) {
            float axisX = 0.0f;
            float axisY = 0.0f;
            float axisZ = 0.0f;
            int axisSign = 1;
            switch (g_animation.face) {
                case FRONT:
                    axisZ = 1.0f;
                    axisSign = 1;
                    break;
                case BACK:
                    axisZ = 1.0f;
                    axisSign = -1;
                    break;
                case LEFT:
                    axisX = 1.0f;
                    axisSign = -1;
                    break;
                case RIGHT:
                    axisX = 1.0f;
                    axisSign = 1;
                    break;
                case UP:
                    axisY = 1.0f;
                    axisSign = 1;
                    break;
                case DOWN:
                    axisY = 1.0f;
                    axisSign = -1;
                    break;
            }
            float angle = g_animation.clockwise ? -g_animation.displayAngle : g_animation.displayAngle;
            angle *= static_cast<float>(axisSign);
            glRotatef(angle, axisX, axisY, axisZ);
        }
        
        glTranslatef(worldX, worldY, worldZ);
        drawCubePiece(piece);
        glPopMatrix();
    }
    
    glPopMatrix();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    
    static int frameCount = 0;
    if (frameCount++ % 30 == 0 && g_logFile != NULL) {
        fprintf(g_logFile, "DISPLAY: frame=%d\n", frameCount);
        fflush(g_logFile);
    }
    
    glTranslatef(0.0f, 0.0f, -CAMERA_DISTANCE);
    rotateAroundAxis(horizontalAxis, cameraAngleY);
    rotateAroundAxis(verticalAxis, cameraAngleX);
    
    drawRubikCube();
    displayTimerOverlay();
    
    glutSwapBuffers();
}

void reshape(int w, int h) {
    g_windowWidth = w;
    g_windowHeight = h;
    
    if (h == 0) {
        h = 1;
    }
    
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    const float fov = 45.0f;
    const float aspect = (float)w / (float)h;
    const float nearPlane = 0.1f;
    const float farPlane = 100.0f;
    
    gluPerspective(fov, aspect, nearPlane, farPlane);
    glMatrixMode(GL_MODELVIEW);
}
