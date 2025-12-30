#ifndef GLOBAL_PROJECT_VARIABLE_HEADER
#define GLOBAL_PROJECT_VARIABLE_HEADER

#include <config.hpp>
#include <types.hpp>

#include <string>
#include <lightObject.hpp>

inline GLFWwindow* mainWindow;

inline ShaderList Shaders;
inline ModelList Models;

inline std::unordered_map<std::string, LightObject*> lightQue;

inline double deltaTime;
inline nanoseconds frameDuration(1'000'000'000 / maxFrameRate); // 1,000,000 μs / 60 = 16666 μs = 16.666 m

inline bool isMinimized = false;
inline bool supressCameraControls = false;
inline bool showMenu = false;
inline bool showScenePicker = true; // has to be TRUE to avoid initial segfaults
inline bool showFPS = false;
inline bool settingsUpdated = true;
inline bool wasStatePausedBeforeMenu = false;
inline bool cartoonColorMode = true;

inline int windowWidth = defaultWindowWidth, windowHeight = defaultWindowHeight;

#endif // GLOBAL_PROJECT_VARIABLE_HEADER