#include <config.hpp>
#include <globals.hpp>

//#include <Timer.hpp>

#include <shader.hpp>
#include <camera.hpp>
#include <UBO.hpp> // Include UBO header

#include <vector>
#include <iostream> // For std::cerr

// Include your OpenGL wrapper classes and the STL loader
#include <VAO.hpp>
#include <VBO.hpp>
#include <EBO.hpp>
//#include <stlImport.hpp> // Include the header for loading STL files
#include <paths.hpp> // Assuming projectPath() is defined here
#include <model.hpp> // Include the Model class header
#include <lightObject.hpp>

#include <simObject.hpp>

#include <scenes.hpp>


inline glm::mat4 calcuculateModelMatrixFromPosition(const glm::vec3& position, const glm::mat4& modelMatrix) {
    return glm::translate(modelMatrix, position);
}
inline glm::mat4 calcuculateModelMatrixFromPosition(const glm::vec3& position) {
    return glm::translate(glm::mat4(1.0f), position);
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Only attempt to render if the model instance has been successfully created

    if (Scenes::currentScene.empty()) {
        if (debugMode) { std::cout << formatError("ERROR") << ": current scene list emtpty... skipping frame." << std::endl; }
        return;
    }

    for (const auto& simObject : Scenes::currentScene) {
        Shader* shader = simObject->shader;
        Model* model = simObject->model;

        shader->activate();
        if (simulateObjectRotation) {
            simObject->modelMatrix = glm::rotate(simObject->modelMatrix, glm::radians(simObject->vertexRotation), glm::vec3(0,1,0)); // temporarily rotate around Z axii
            shader->applyModelMatrix( calcuculateModelMatrixFromPosition(simObject->position) * simObject->modelMatrix );
        }
        else {
            shader->applyModelMatrix(calcuculateModelMatrixFromPosition(simObject->position));
        }

        simObject->draw();
    }

    glfwSwapBuffers(mainWindow);
}

void setupShaderMetrices(Shader* shader);

Camera* currentCamera;

// Global UBO for light properties
UBO* lightBlockUBO = nullptr;

// Define MAX_LIGHTS to match the shader
#define MAX_LIGHTS 4

const GLuint LIGHT_UBO_BINDING_POINT = 0; // Choose a binding point for the LightBlock UBO

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

// Function to initialize the model data and OpenGL buffers for the main model
void setupModels() {

    std::set<std::string> availableFormats = getSupportedAssimpExtensions();

    for (const auto& file : std::filesystem::recursive_directory_iterator(projectPath(modelPath))) {
        

        if (availableFormats.find(file.path().extension().string()) != availableFormats.end()) {
            auto filepath = file.path();
            Models[filepath.stem().string()] = new Model(loadSTLData(filepath), glm::vec3(1.0f, 1.0f, 1.0f)); // White by default
        }
    }

    // Create an instance of your Model class
    //mainModelInstance = new Model(projectPath("res/models/pyramid.stl"), glm::vec3(1.0f, 1.0f, 1.0f)); // White by default

    // Initialize the LightBlock UBO
    // The size should be calculated based on the struct LightBlockData
    lightBlockUBO = new UBO(sizeof(LightBlockData));

    // shaders that do not have a setup for light block will be ignored
    for (const auto& shader : Shaders) {
        shader.second->activate();
        shader.second->setUniformBlockBinding("LightBlock", LIGHT_UBO_BINDING_POINT);
    }
}

void setupShaders() {
    struct shaderSource {
        std::filesystem::path vertex;
        std::filesystem::path fragment;
    };

    std::unordered_map<std::string, shaderSource> shaderSourceFiles;

    // loading and sorting source files
    for (const auto& file : std::filesystem::recursive_directory_iterator(projectPath(shaderPath))) {
        auto filepath = file.path();
        if (filepath.extension() == ".vert") {
            shaderSourceFiles[filepath.stem().string()].vertex = filepath;
        }
        else if (filepath.extension() == ".frag") {
            shaderSourceFiles[filepath.stem().string()].fragment = filepath;
        }
    }

    // attempting to make shaders from source files
    TinyInt failed = 0;

    for (const auto& shaderSource : shaderSourceFiles) {
        if (!shaderSource.second.vertex.empty() && !shaderSource.second.fragment.empty()) {
            Shaders[shaderSource.first] = new Shader(shaderSource.second.vertex, shaderSource.second.fragment);
            setupShaderMetrices(Shaders[shaderSource.first]);
        }
        else { failed++; }
    }

    if (debugMode) {
        std::cout << "\n" << formatProcess("Loaded ") << Shaders.size() << " Shader" << ((Shaders.size() > 1u) ? "s" : "") << ((failed > 0u) ? "; failed " + failed : "") << std::endl;
    }
}

// Function to clean up all dynamically allocated model resources
void cleanupRender() {
    if (lightBlockUBO) {
        delete lightBlockUBO;
        lightBlockUBO = nullptr;
    }
}

void updateLightSourcePositions() {
    for (const auto& light : lightQue) {
        if (SimObjects.find(light.first) != SimObjects.end()) {
            light.second->position = SimObjects[light.first]->position;
        }
    }
}

// prototype function; later will be culling based on distance
void updateLightSources() {
    updateLightSourcePositions();

    int amountOfLights = std::min(MAX_LIGHTS, (int)lightQue.size());

    LightBlockData lightsData;
    lightsData.lightCount = amountOfLights;
    lightsData.lightFallOff = lightFalloff;

    unsigned char lightsAdded = 0;
    for (const auto& light : lightQue) {
        if (lightsAdded >= MAX_LIGHTS) { break; }

        lightsData.lights[lightsAdded].position = light.second->position;
        lightsData.lights[lightsAdded].color = glm::vec4(light.second->color, 1);
        lightsData.lights[lightsAdded].intensity = light.second->intensity;

        lightsAdded++;
    }

    // Update the UBO with the new light data
    lightBlockUBO->update(0, sizeof(LightBlockData), &lightsData);

    // Bind the UBO to the specified binding point
    lightBlockUBO->bind(LIGHT_UBO_BINDING_POINT);
}

/* one shot temporary render setup
void renderSetup() {
    lightQue["sol"] = new LightObject(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.988, 0.658, 0.011), 1.5f); // sun light


    updateLightSources();
}*/

void resize(GLFWwindow *window, int width, int height) {
    // Ensure the mainShader is active before updating projection
    for (const auto& shaderPair : Shaders) {
        auto shader = shaderPair.second;
        if (shader) {
            shader->activate();
            currentCamera->updateProjection(width, height, shader);
        }
    }
}

void setupShaderMetrices(Shader* shader) {
    int windowWidth, windowHeight;
    glfwGetFramebufferSize(mainWindow, &windowWidth, &windowHeight);

    shader->activate();

    // Initialize the camera (make sure it's only initialized once)
    if (!currentCamera) {
        currentCamera = new Camera(windowWidth, windowHeight, glm::vec3(0.0f, 0.5f, 2.0f));
    }

    currentCamera->updateProjection(windowWidth, windowHeight, shader);

    // Initial application of the model matrix (can be overridden in render loop)
    shader->applyModelMatrix();
}

void APIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
    GLsizei length, const GLchar *message, const void *userParam) {
    std::cout << formatError("OpenGL Debug") << ": " << colorText(message, ANSII_YELLOW) << std::endl;
}