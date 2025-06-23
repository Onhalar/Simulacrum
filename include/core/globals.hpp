#ifndef GLOBAL_PROJECT_VARIABLE_HEADER
#define GLOBAL_PROJECT_VARIABLE_HEADER

#include <config.hpp>
#include <shader.hpp>

GLFWwindow* mainWindow;

Shader* mainShader;
Shader* lightShader;

double deltaTime;
nanoseconds frameDuration(1'000'000'000 / maxFrameRate); // 1,000,000 μs / 60 = 16666 μs = 16.666 m

#endif // GLOBAL_PROJECT_VARIABLE_HEADER