#ifndef GLOBAL_PROJECT_VARIABLE_HEADER
#define GLOBAL_PROJECT_VARIABLE_HEADER

#include <config.hpp>
#include <shader.hpp>
#include <unordered_map>
#include <string>

#include <shader.hpp>
#include <model.hpp>

using ShaderList = std::unordered_map<std::string, Shader*>;
using ModelList = std::unordered_map<std::string, Model*>;

GLFWwindow* mainWindow;

ShaderList Shaders;
ModelList Models;

double deltaTime;
nanoseconds frameDuration(1'000'000'000 / maxFrameRate); // 1,000,000 μs / 60 = 16666 μs = 16.666 m

#endif // GLOBAL_PROJECT_VARIABLE_HEADER