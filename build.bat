@echo off
echo ========================================
echo Rubik's Cube - Build and Run
echo ========================================
echo.

echo Compiling all modules...
g++ -std=c++98 -Wall -Wextra -O2 src\main.cpp src\rubik_state.cpp src\rubik_rotation.cpp src\rubik_animation.cpp src\rubik_timer.cpp src\rubik_input.cpp src\rubik_render.cpp -Iinclude -I"C:\mingw64\include" -L"C:\mingw64\lib" -lfreeglut -lopengl32 -lglu32 -o build\rubik.exe

if %errorlevel% neq 0 (
    echo.
    echo [ERROR] Compilation failed!
    pause
    exit /b %errorlevel%
)

echo.
echo [SUCCESS] Compilation successful!
echo.
echo Running Rubik's Cube...
echo ========================================
echo.

build\rubik.exe

echo.
echo ========================================
echo Program terminated.
pause
