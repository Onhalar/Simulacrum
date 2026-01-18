#ifndef MAIN_CONFIG_HEADER
#define MAIN_CONFIG_HEADER

#include <unordered_map>

#include <filesystem>
#include <string>

#include <chrono>

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
inline filesystem::path projectDir;

// main icon path
inline string iconPath = "res/icon.png";

// resource paths
inline const std::filesystem::path shaderPath = "shaders/";
inline const std::filesystem::path resourcePath = "res/";

inline const std::filesystem::path modelPath = resourcePath/"models";
inline const std::filesystem::path settingsPath = resourcePath/"settings.conf";
inline const std::filesystem::path simObjectsConfigPath = resourcePath/"objects.json";
inline const std::filesystem::path physicsScenesPath = resourcePath/"scenes.json";


// window settings
inline int defaultWindowWidth = 500;
inline int defaultWindowHeight = 500;

inline int minWindowWidth = 300;
inline int minWindowHeight = 300;

inline Color backgroundColor(0.07f, 0.13f, 0.17f, 1.0f);

inline const char* windowName = "Simulacrum project";

// render settings
inline int maxFrameRate = 60;
inline int VSync = 1;

inline bool doPostProcess = true;
inline bool doFXAA = true;
inline bool inverseColors = false;
inline bool renderUnsimulated = false;

inline float ambientStrength = 0.2;

// flag signaling whether the main window is or is not fullscreened. DO NOT MODIFY - handled by enter / exit fullscreen functions.
inline bool fullscreen = false;

inline float staticDelayFraction = 0.65f;
inline bool simulateObjectRotation = true;

inline nanoseconds spinDelay(375); // about 350 - 400 ns
inline TinyInt lightUpdateFrameSkip = 2;

inline float renderDistance = 1'000.0f;

inline float cameraSpeed = 12.5f;
inline float cameraSensitivity = 175.0f;
inline float fovDeg = 45.0f;

inline float sceneZoomModifier = 0.05;

inline double unifiedDistance = 10.0; // used for simplified mode scaling


// GUI
inline float fontSize = 15.0f;
inline float windowRounding = 8.0f;
inline float frameRounding = 4.0f;
inline std::string fontFile = "mandalore.otf";

// Model settings

inline unsigned int starScaleMultiplier = 20.0f;
inline float normalizedModelRadius = 1.0f;
inline simulationType simulationMode = simulationType::simplified;
inline float maxScale = 6.0f;
inline double currentScale; // kilometers per vertex - only for distances; use already present values for all else (especially for simplified mode)
inline double renderScaleDistortion = 1.0; // 1.0 -> no distortion; less -> greater distances; more -> smaller distances


inline unsigned int phyiscsSubsteps = 2;

// functions

inline std::string projectPath(const std::string& path) {
    return (projectDir / std::filesystem::path(path)).string();
}
inline std::string projectPath(const std::filesystem::path& path) {
    return (projectDir / path).string();
}

// physics
inline double simulationSpeed = 2.5e5;
inline float physicsSteps = 60.0f; // amount of physics steps per second
inline bool gravityInInitialVel = false;
inline bool trackSimTime = true;

#define PI 3.141592653589793
#define GRAVITATIONAL_CONSTANT 6.6743e-11 // m³ kg⁻¹ s⁻²

#endif // MAIN_CONFIG_HEADER