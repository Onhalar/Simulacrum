#include <config.hpp>
#include <globals.hpp>

#include <Timer.hpp>

#include <shader.hpp>
#include <camera.hpp>
#include <UBO.hpp> // Include UBO header

#include <vector>
#include <iostream> // For std::cerr

// Include your OpenGL wrapper classes and the STL loader
#include <VAO.hpp>
#include <VBO.hpp>
#include <EBO.hpp>
#include <stlImport.hpp> // Include the header for loading STL files
#include <paths.hpp> // Assuming projectPath() is defined here
#include <model.hpp> // Include the Model class header
#include <lightObject.hpp>

vector<LightObject*> lightQue;

Camera* currentCamera;

// Global pointer for the loaded model instance
Model* mainModelInstance = nullptr;

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
    float lightDistance;
    float padding[2];
};


// Function to initialize the model data and OpenGL buffers for the main model
void setupModels() {
    // Create an instance of your Model class
    mainModelInstance = new Model(projectPath("res/models/pyramid.stl"), glm::vec3(1.0f, 1.0f, 1.0f)); // White by default

    // Initialize the LightBlock UBO
    // The size should be calculated based on the struct LightBlockData
    lightBlockUBO = new UBO(sizeof(LightBlockData));

    // Set the uniform block binding for the shader
    // This connects the 'LightBlock' in the shader to LIGHT_UBO_BINDING_POINT
    if (mainShader) {
        mainShader->activate();
        mainShader->setUniformBlockBinding("LightBlock", LIGHT_UBO_BINDING_POINT);
    }
}

// Function to clean up all dynamically allocated model resources
void cleanup() {
    if (mainModelInstance) {
        delete mainModelInstance;
        mainModelInstance = nullptr;
    }
    if (lightBlockUBO) {
        lightBlockUBO->destroy();
        delete lightBlockUBO;
        lightBlockUBO = nullptr;
    }
}

// prototype function; later will be culling based on distance
void updateLightSources() {
    int amountOfLights = min(MAX_LIGHTS, (int)lightQue.size());

    LightBlockData lightsData;
    lightsData.lightCount = amountOfLights;

    for (int i = 0; i < amountOfLights; i++) {
        lightsData.lights[i].position = lightQue[i]->position;
        lightsData.lights[i].color = glm::vec4(lightQue[i]->color, 1);
        lightsData.lights[i].intensity = lightQue[i]->intensity;
    }

    // Update the UBO with the new light data
    lightBlockUBO->update(0, sizeof(LightBlockData), &lightsData);

    // Bind the UBO to the specified binding point
    lightBlockUBO->bind(LIGHT_UBO_BINDING_POINT);
}

// one shot temporary render setup
void renderSetup() {
    lightQue.push_back(new LightObject(glm::vec3(5,2,5), glm::vec3(0.0f, 1.0f, 0.0f), 0.2f));
    lightQue.push_back(new LightObject(glm::vec3(5,-2,5), glm::vec3(0.0f, 0.0f, 1.0f), 0.4f));

    updateLightSources();
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Only attempt to render if the model instance has been successfully created
    if (mainModelInstance && mainShader && lightBlockUBO) {
        // Define a model matrix to position and orient the model in the scene
        glm::mat4 modelMatrix = glm::mat4(1.0f); // Start with an identity matrix

        // Apply rotation to correct orientation (e.g., if model is Z-up and OpenGL is Y-up)
        modelMatrix = glm::rotate(modelMatrix, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate around X-axis

        // Translate the model back so it's visible in front of the camera
        //modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, -2.0f));

        // Get camera position from your Camera class
        glm::vec3 cameraPosition = currentCamera->position;

        // Activate the shader
        mainShader->activate();

        // Set the camera position uniform (still needed as it's not in the UBO)
        mainShader->setUniform("cameraPosition", cameraPosition);

        mainModelInstance->draw(mainShader, modelMatrix, cameraPosition);
    }

    glfwSwapBuffers(mainWindow);
}

void resize(GLFWwindow *window, int width, int height) {
    // Ensure the mainShader is active before updating projection
    if (mainShader) {
        mainShader->activate();
        currentCamera->updateProjection(width, height, mainShader);
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