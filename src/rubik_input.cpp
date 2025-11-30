#include "rubik_input.h"
#include "rubik_state.h"
#include "rubik_rotation.h"
#include "rubik_animation.h"
#include "rubik_constants.h"
#include <GL/glut.h>
#include <cstdio>
#include <cctype>
#include <cmath>

float cameraAngleX = 0.0f;
float cameraAngleY = 0.0f;
bool isDragging = false;
int lastMouseX = 0;
int lastMouseY = 0;
Face currentFrontFace = FRONT;
float verticalAxis[3] = {0.0f, 0.0f, 0.0f};
float horizontalAxis[3] = {0.0f, 0.0f, 0.0f};

void updateRotationAxes() {
    verticalAxis[0] = 0.0f;
    verticalAxis[1] = 0.0f;
    verticalAxis[2] = 0.0f;
    horizontalAxis[0] = 0.0f;
    horizontalAxis[1] = 0.0f;
    horizontalAxis[2] = 0.0f;
    
    switch (currentFrontFace) {
        case FRONT:
            verticalAxis[0] = 1.0f;
            horizontalAxis[1] = 1.0f;
            break;
        case RIGHT:
            verticalAxis[2] = 1.0f;
            horizontalAxis[0] = 1.0f;
            break;
        case BACK:
            verticalAxis[0] = -1.0f;
            horizontalAxis[1] = -1.0f;
            break;
        case LEFT:
            verticalAxis[2] = -1.0f;
            horizontalAxis[0] = -1.0f;
            break;
        case UP:
            verticalAxis[1] = 1.0f;
            horizontalAxis[2] = 1.0f;
            break;
        case DOWN:
            verticalAxis[1] = -1.0f;
            horizontalAxis[2] = -1.0f;
            break;
    }
    
    if (g_logFile != NULL) {
        const char* faceNames[] = {"FRONT", "BACK", "LEFT", "RIGHT", "UP", "DOWN"};
        fprintf(g_logFile, "FACE CHANGE: %s | verticalAxis=[%.1f,%.1f,%.1f] horizontalAxis=[%.1f,%.1f,%.1f]\n",
                faceNames[currentFrontFace],
                verticalAxis[0], verticalAxis[1], verticalAxis[2],
                horizontalAxis[0], horizontalAxis[1], horizontalAxis[2]);
        fflush(g_logFile);
    }
}

void resetRotationAngles() {
    cameraAngleX = 0.0f;
    cameraAngleY = 0.0f;
}

void applyCurrentViewRotation(float& x, float& y, float& z) {
    rotateVectorAroundAxis(verticalAxis, cameraAngleX, x, y, z);
    rotateVectorAroundAxis(horizontalAxis, cameraAngleY, x, y, z);
}

void computeViewFaceMapping(ViewFaceMapping& mapping) {
    const float normals[6][3] = {
        {0.0f, 0.0f, 1.0f},   // FRONT
        {0.0f, 0.0f, -1.0f},  // BACK
        {-1.0f, 0.0f, 0.0f},  // LEFT
        {1.0f, 0.0f, 0.0f},   // RIGHT
        {0.0f, 1.0f, 0.0f},   // UP
        {0.0f, -1.0f, 0.0f}   // DOWN
    };
    const Face faces[6] = {FRONT, BACK, LEFT, RIGHT, UP, DOWN};
    float rotated[6][3];
    for (int i = 0; i < 6; i++) {
        rotated[i][0] = normals[i][0];
        rotated[i][1] = normals[i][1];
        rotated[i][2] = normals[i][2];
        applyCurrentViewRotation(rotated[i][0], rotated[i][1], rotated[i][2]);
    }
    float bestFrontDot = -1000.0f;
    float bestUpDot = -1000.0f;
    int frontIdx = 0;
    int upIdx = 4;
    const float viewFront[3] = {0.0f, 0.0f, 1.0f};
    const float viewUp[3] = {0.0f, 1.0f, 0.0f};
    for (int i = 0; i < 6; i++) {
        float dotFront = rotated[i][0] * viewFront[0] + rotated[i][1] * viewFront[1] + rotated[i][2] * viewFront[2];
        if (dotFront > bestFrontDot) {
            bestFrontDot = dotFront;
            frontIdx = i;
        }
    }
    for (int i = 0; i < 6; i++) {
        if (i == frontIdx || faces[i] == getOppositeFace(faces[frontIdx])) {
            continue;
        }
        float dotUp = rotated[i][0] * viewUp[0] + rotated[i][1] * viewUp[1] + rotated[i][2] * viewUp[2];
        if (dotUp > bestUpDot) {
            bestUpDot = dotUp;
            upIdx = i;
        }
    }
    float frontVec[3] = {rotated[frontIdx][0], rotated[frontIdx][1], rotated[frontIdx][2]};
    float upVec[3] = {rotated[upIdx][0], rotated[upIdx][1], rotated[upIdx][2]};
    float frontDotUp = frontVec[0] * upVec[0] + frontVec[1] * upVec[1] + frontVec[2] * upVec[2];
    if (fabs(frontDotUp) > 0.9f) {
        bestUpDot = -1000.0f;
        for (int i = 0; i < 6; i++) {
            if (i == frontIdx || faces[i] == getOppositeFace(faces[frontIdx])) {
                continue;
            }
            float dotUp = rotated[i][0] * viewUp[0] + rotated[i][1] * viewUp[1] + rotated[i][2] * viewUp[2];
            if (dotUp > bestUpDot && fabs(rotated[i][0] * frontVec[0] + rotated[i][1] * frontVec[1] + rotated[i][2] * frontVec[2]) < 0.1f) {
                bestUpDot = dotUp;
                upIdx = i;
                upVec[0] = rotated[i][0];
                upVec[1] = rotated[i][1];
                upVec[2] = rotated[i][2];
            }
        }
    }
    float rightVec[3];
    rightVec[0] = upVec[1] * frontVec[2] - upVec[2] * frontVec[1];
    rightVec[1] = upVec[2] * frontVec[0] - upVec[0] * frontVec[2];
    rightVec[2] = upVec[0] * frontVec[1] - upVec[1] * frontVec[0];
    float rightLen = sqrt(rightVec[0] * rightVec[0] + rightVec[1] * rightVec[1] + rightVec[2] * rightVec[2]);
    if (rightLen > 0.0001f) {
        rightVec[0] /= rightLen;
        rightVec[1] /= rightLen;
        rightVec[2] /= rightLen;
    } else {
        rightVec[0] = 1.0f;
        rightVec[1] = 0.0f;
        rightVec[2] = 0.0f;
    }
    int rightIdx = 3;
    float bestRightDot = -1000.0f;
    for (int i = 0; i < 6; i++) {
        float dotRight = rotated[i][0] * rightVec[0] + rotated[i][1] * rightVec[1] + rotated[i][2] * rightVec[2];
        if (dotRight > bestRightDot) {
            bestRightDot = dotRight;
            rightIdx = i;
        }
    }
    mapping.front = faces[frontIdx];
    mapping.back = getOppositeFace(mapping.front);
    mapping.up = faces[upIdx];
    mapping.down = getOppositeFace(mapping.up);
    mapping.right = faces[rightIdx];
    mapping.left = getOppositeFace(mapping.right);
}

bool performRelativeFaceTurn(int relativeFace, bool clockwise) {
    Face absoluteFace = getAbsoluteFace(relativeFace);
    startRotation(absoluteFace, clockwise);
    return true;
}

void mouse(int button, int state, int x, int y) {
    if (g_logFile != NULL) {
        fprintf(g_logFile, "MOUSE EVENT: button=%d state=%d x=%d y=%d\n", button, state, x, y);
        fflush(g_logFile);
    }
    
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            if (g_logFile != NULL) {
                fprintf(g_logFile, "*** DRAG START ***\n");
                fflush(g_logFile);
            }
            isDragging = true;
            lastMouseX = x;
            lastMouseY = y;
        } else if (state == GLUT_UP) {
            if (g_logFile != NULL) {
                fprintf(g_logFile, "*** DRAG END ***\n");
                fflush(g_logFile);
            }
            isDragging = false;
        }
    }
    glutPostRedisplay();
}

void motion(int x, int y) {
    if (!isDragging) {
        return;
    }
    
    int dx = x - lastMouseX;
    int dy = y - lastMouseY;
    
    float yawDelta = (float)dx * ROTATION_SENSITIVITY;
    float pitchDelta = (float)(y - lastMouseY) * ROTATION_SENSITIVITY;
    
    if (g_logFile != NULL) {
        fprintf(g_logFile, "MOUSE: x=%d y=%d | dx=%d dy=%d | yawDelta=%.1f pitchDelta=%.1f\n", 
                x, y, dx, dy, yawDelta, pitchDelta);
        fflush(g_logFile);
    }
    
    cameraAngleY += yawDelta;
    cameraAngleX += pitchDelta;
    
    if (g_logFile != NULL) {
        fprintf(g_logFile, "  → Updated angles: X=%.1f Y=%.1f\n", cameraAngleX, cameraAngleY);
        fflush(g_logFile);
    }
    
    lastMouseX = x;
    lastMouseY = y;
    glutPostRedisplay();
}

void keyboard(unsigned char key, int /* x */, int /* y */) {
    int modifiers = glutGetModifiers();
    bool shiftDown = (modifiers & GLUT_ACTIVE_SHIFT) != 0;
    int keyUpper = toupper((unsigned char)key);
    bool trackKey = false;
    switch (keyUpper) {
        case 'F':
        case 'U':
        case 'R':
        case 'L':
        case 'D':
        case 'B':
        case 'S':
            trackKey = true;
            break;
        default:
            break;
    }
    if (key == ' ') {
        trackKey = true;
        keyUpper = ' ';
    }
    if (trackKey) {
        unsigned char idx = (unsigned char)keyUpper;
        if (g_keyHeld[idx]) {
            return;
        }
        g_keyHeld[idx] = true;
    }
    Face newFace = currentFrontFace;
    bool faceChanged = false;
    
    switch (keyUpper) {
        case ' ':
            resetCube();
            glutPostRedisplay();
            return;
            
        case 'S':
            shuffleCube(20);
            glutPostRedisplay();
            return;
            
        case 'F':
            performRelativeFaceTurn(0, !shiftDown);
            return;
            
        case 'U':
            performRelativeFaceTurn(1, !shiftDown);
            return;
            
        case 'R':
            performRelativeFaceTurn(2, !shiftDown);
            return;
            
        case 'L':
            performRelativeFaceTurn(3, !shiftDown);
            return;
            
        case 'D':
            performRelativeFaceTurn(4, !shiftDown);
            return;
            
        case 'B':
            performRelativeFaceTurn(5, !shiftDown);
            return;
            
        case 'f':
            newFace = FRONT;
            faceChanged = true;
            break;
            
        case 'r':
            newFace = RIGHT;
            faceChanged = true;
            break;
            
        case 'b':
            newFace = BACK;
            faceChanged = true;
            break;
            
        case 'l':
            newFace = LEFT;
            faceChanged = true;
            break;
            
        case 'u':
            newFace = UP;
            faceChanged = true;
            break;
            
        case 'd':
            newFace = DOWN;
            faceChanged = true;
            break;
            
        default:
            break;
    }
    
    switch (key) {
        case 'f':
            newFace = FRONT;
            faceChanged = true;
            break;
            
        case 'r':
            newFace = RIGHT;
            faceChanged = true;
            break;
            
        case 'b':
            newFace = BACK;
            faceChanged = true;
            break;
            
        case 'l':
            newFace = LEFT;
            faceChanged = true;
            break;
            
        case 'u':
            newFace = UP;
            faceChanged = true;
            break;
            
        case 'd':
            newFace = DOWN;
            faceChanged = true;
            break;
        default:
            break;
    }
    
    if (faceChanged) {
        currentFrontFace = newFace;
        updateRotationAxes();
        glutPostRedisplay();
    }
}

void keyboardUp(unsigned char key, int /* x */, int /* y */) {
    int keyUpper = toupper((unsigned char)key);
    if (keyUpper < 0 || keyUpper >= 256) {
        return;
    }
    g_keyHeld[(unsigned char)keyUpper] = false;
    if (key == ' ') {
        g_keyHeld[(unsigned char)' '] = false;
    }
}

void keyboardSpecial(int key, int /* x */, int /* y */) {
    const float ROTATION_STEP = KEYBOARD_ROTATION_SPEED;
    const char* keyName = "";
    
    switch (key) {
        case GLUT_KEY_UP:
            cameraAngleX -= ROTATION_STEP;
            keyName = "UP";
            break;
            
        case GLUT_KEY_DOWN:
            cameraAngleX += ROTATION_STEP;
            keyName = "DOWN";
            break;
            
        case GLUT_KEY_LEFT:
            cameraAngleY -= ROTATION_STEP;
            keyName = "LEFT";
            break;
            
        case GLUT_KEY_RIGHT:
            cameraAngleY += ROTATION_STEP;
            keyName = "RIGHT";
            break;
            
        default:
            return;
    }
    
    if (g_logFile != NULL) {
        fprintf(g_logFile, "KEYBOARD: %s pressed | angleX=%.1f angleY=%.1f\n", keyName, cameraAngleX, cameraAngleY);
        fflush(g_logFile);
    }
    
    glutPostRedisplay();
}
