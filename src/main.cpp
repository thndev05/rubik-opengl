/*
 * Rubik's Cube - 3x3x3 Rubik's Cube Simulator
 * Computer Graphics Final Project
 * 
 * This is a modular implementation of a Rubik's Cube simulator with:
 * - Full 3x3x3 cube with 27 pieces
 * - Smooth animations with easing
 * - Camera rotation controls (mouse drag & arrow keys)
 * - Face rotation controls (F/U/R/L/D/B keys)
 * - Timer and speedsolving features
 * - Auto-scramble functionality
 * 
 * Compilation (Windows/MinGW - PowerShell):
 * g++ -std=c++98 -Wall -Wextra -O2 main.cpp rubik_state.cpp rubik_rotation.cpp rubik_animation.cpp rubik_timer.cpp rubik_input.cpp rubik_render.cpp -lfreeglut -lopengl32 -lglu32 -o rubik.exe
 * 
 * Or one-line compile + run:
 * g++ -std=c++98 -Wall -Wextra -O2 main.cpp rubik_state.cpp rubik_rotation.cpp rubik_animation.cpp rubik_timer.cpp rubik_input.cpp rubik_render.cpp -lfreeglut -lopengl32 -lglu32 -o rubik.exe; if ($?) { .\rubik.exe }
 * 
 * Compilation (Linux):
 * g++ -std=c++98 -Wall -Wextra -O2 main.cpp rubik_state.cpp rubik_rotation.cpp rubik_animation.cpp rubik_timer.cpp rubik_input.cpp rubik_render.cpp -lglut -lGLU -lGL -lm -o rubik
 * 
 * Controls:
 * - Mouse drag: Rotate camera view
 * - Arrow keys: Rotate camera view
 * - F/U/R/L/D/B: Rotate Front/Up/Right/Left/Down/Back face clockwise
 * - Shift + F/U/R/L/D/B: Rotate face counter-clockwise
 * - S: Scramble cube (20 random moves)
 * - Space: Reset cube to solved state
 */

#include <GL/glut.h>
#include <cstdlib>
#include <ctime>
#include <iostream>

// Include all module headers
#include "rubik_types.h"
#include "rubik_constants.h"
#include "rubik_state.h"
#include "rubik_rotation.h"
#include "rubik_animation.h"
#include "rubik_timer.h"
#include "rubik_input.h"
#include "rubik_render.h"

int main(int argc, char** argv) {
    // Initialize debug log file
    initLogFile();
    
    // Initialize GLUT
    glutInit(&argc, argv);
    
    // Set display mode
    unsigned int displayMode = GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH;
#ifdef GLUT_MULTISAMPLE
    displayMode |= GLUT_MULTISAMPLE;
#endif
    glutInitDisplayMode(displayMode);
    
    // Set window size and position
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);
    
    // Create window
    glutCreateWindow("Rubik's Cube - 3x3x3 Simulator");
    
    // Initialize OpenGL settings
    initOpenGL();
    
    // Initialize Rubik's Cube
    initRubikCube();
    
    // Test rotation identity
    testRotationIdentity();
    
    // Initialize rotation axes
    updateRotationAxes();
    
    // Initialize random seed for shuffle
    srand((unsigned int)time(NULL));
    
    // Register callback functions
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(keyboardSpecial);
    glutIdleFunc(idle);
    glutIgnoreKeyRepeat(1);
    
    g_lastTimeMs = glutGet(GLUT_ELAPSED_TIME);
    
    // Log initialization complete
    if (g_logFile != NULL) {
        fprintf(g_logFile, "Application initialized successfully\n\n");
        fflush(g_logFile);
    }
    
    std::cout << "=== Rubik's Cube Simulator ===" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  Mouse drag: Rotate camera" << std::endl;
    std::cout << "  Arrow keys: Rotate camera" << std::endl;
    std::cout << "  F/U/R/L/D/B: Rotate faces (Shift for counter-clockwise)" << std::endl;
    std::cout << "  S: Scramble (20 random moves)" << std::endl;
    std::cout << "  Space: Reset to solved state" << std::endl;
    std::cout << "==============================\n" << std::endl;
    
    // Enter GLUT main loop
    glutMainLoop();
    
    // Close log file before exit
    closeLogFile();
    
    return 0;
}
