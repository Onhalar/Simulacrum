#ifndef MAIN_CONFIG_HEADER
#define MAIN_CONFIG_HEADER

#include <iostream>
#include <map>
#include <unordered_map>
#include <vector>

#include <filesystem>
#include <string>
#include <fstream>
#include <sstream>

#include <chrono>
#include <thread>

#include <any>
#include <typeindex>
#include <tuple>
#include <utility>
#include <set>

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <types.hpp>

#include <shader.hpp>
#include <model.hpp>

using ShaderList = std::unordered_map<std::string, Shader*>;
using ModelList = std::unordered_map<std::string, Model*>;

using namespace std;
using namespace chrono;

// project directory
filesystem::path projectDir;

// main icon path
string iconPath = "res/icon.png";

// resource paths
const std::filesystem::path shaderPath = "shaders/";
const std::filesystem::path modelPath = "res/models";
const std::filesystem::path settingsPath = "res/settings.json";
const std::filesystem::path simObjectsConfigPath = "res/objects.json";
const std::filesystem::path physicsScenesPath = "res/scenes.json";


// window settings
int defaultWindowWidth = 500;
int defaultWindowHeight = 500;

int minWindowWidth = 300;
int minWindowHeight = 300;

Color defaultBackgroundColor = { 0.07f, 0.13f, 0.17f, 1.0f };

const char* windowName = "Simulacrum project";

// render settings
int maxFrameRate = 60;
int VSync = 1;

float staticDelayFraction = 0.65f;
bool simulateObjectRotation = true;

nanoseconds spinDelay(375); // about 350 - 400 ns
TinyInt lightUpdateFrameSkip = 2;

float renderDistance = 1'000.0f;

float cameraSpeed = 12.5f;
float cameraSensitivity = 175.0f;

// Model settings

float normalizedModelRadius = 0.85f;
simulationType simulationMode = simulationType::simplified;
float maxScale = 7.5f;
double currentScale; // kilometers per vertex - only for distances; use already present values for all else (especially for simplified mode)
double renderScaleDistortion = 1.0; // 1.0 -> no distortion; less -> greater distances; more -> smaller distances
unsigned int phyiscsSubsteps = 2;
unsigned int phyiscsBufferedFrames = 1;

// functions

inline std::string projectPath(const std::string& path) {
    return (projectDir / std::filesystem::path(path)).string();
}
inline std::string projectPath(const std::filesystem::path& path) {
    return (projectDir / path).string();
}

// physics
double simulationSpeed = 5.0e3;

#define PI 3.141592653589793
#define GRAVITATIONAL_CONSTANT 6.6743e-11 // m³ kg⁻¹ s⁻²

#endif // MAIN_CONFIG_HEADER