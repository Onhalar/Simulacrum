#ifndef MAIN_CONFIG_HEADER
#define MAIN_CONFIG_HEADER

#include <iostream>
#include <map>
#include <vector>

#include <filesystem>
#include <string>
#include <fstream>
#include <sstream>

#include <tuple>
#include <utility>

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

// project directory
filesystem::path projectDir;

// main icon path
const char* iconPath = "res/icon.png";

// setup settings
const bool debugMode = true;

// window settings
const int defaultWindowWidth = 500;
const int defaultWindowHeight = 500;

const int minWindowWidth = 300;
const int minWindowHeight = 300;

const tuple defaultBackgroundColor = { 0.07f, 0.13f, 0.17f, 1.0f };

const char* windowName = "Simulacrum project";

// render settings
int maxFrameRate = 60;

#endif // MAIN_CONFIG_HEADER