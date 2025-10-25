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

#endif // RENDER_DEFINITIONS