#ifndef GLOBAL_PROJECT_VARIABLE_HEADER
#define GLOBAL_PROJECT_VARIABLE_HEADER

#include <config.hpp>
#include <types.hpp>

#include <string>
#include <lightObject.hpp>

GLFWwindow* mainWindow;

ShaderList Shaders;
ModelList Models;

std::unordered_map<std::string, LightObject*> lightQue;

double deltaTime;
nanoseconds frameDuration(1'000'000'000 / maxFrameRate); // 1,000,000 μs / 60 = 16666 μs = 16.666 m

bool isMinimized = false;

#endif // GLOBAL_PROJECT_VARIABLE_HEADER