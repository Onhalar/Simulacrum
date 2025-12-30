#ifndef RENDER_DEFINITIONS
#define RENDER_DEFINITIONS

#include <config.hpp>
#include <UBO.hpp>
#include <camera.hpp>
#include <FBO.hpp>
#include <unordered_map>

using FBOList = unordered_map<std::string, FBO*>;

// global shader settings
inline float lightFalloff = 0.001f;
#define MAX_LIGHTS 4 // Define MAX_LIGHTS to match the shader

// Global UBO for light properties
inline UBO* lightBlockUBO = nullptr;

const GLuint LIGHT_UBO_BINDING_POINT = 0; // Choose a binding point for the LightBlock UBO

inline Camera* currentCamera;

inline FBOList FBOs;

struct ShaderLight {
    glm::vec3 position;
    float padding1;
    glm::vec4 color;
    float intensity;
    float padding2[3];
};

struct LightBlockData {
    ShaderLight lights[MAX_LIGHTS]; // Array of lights
    int lightCount;
    float lightFallOff;
    float padding[2];
};

inline std::map<char, glm::vec3> starTypeCartoonEmissions = {
    {'O', {0.10f, 0.30f, 1.00f}},
    {'B', {0.20f, 0.60f, 1.00f}},
    {'A', {0.70f, 0.90f, 1.00f}},
    {'F', {1.00f, 1.00f, 0.80f}},
    {'G', {1.00f, 0.85f, 0.00f}},
    {'K', {1.00f, 0.40f, 0.00f}},
    {'M', {1.00f, 0.00f, 0.00f}}
};

void enterFullscreen();
void exitFullscreen();

#endif // RENDER_DEFINITIONS