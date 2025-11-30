#ifndef RUBIK_INPUT_H
#define RUBIK_INPUT_H

#include "rubik_types.h"

// Camera control
extern float cameraAngleX;
extern float cameraAngleY;
extern bool isDragging;
extern int lastMouseX;
extern int lastMouseY;
extern Face currentFrontFace;
extern float verticalAxis[3];
extern float horizontalAxis[3];

// Input callbacks
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void keyboard(unsigned char key, int x, int y);
void keyboardUp(unsigned char key, int x, int y);
void keyboardSpecial(int key, int x, int y);

// View management
void updateRotationAxes();
void resetRotationAngles();
void computeViewFaceMapping(ViewFaceMapping& mapping);
void applyCurrentViewRotation(float& x, float& y, float& z);
bool performRelativeFaceTurn(int relativeFace, bool clockwise);

#endif // RUBIK_INPUT_H
