#include <config.hpp>
#include <globals.hpp>

#include <Timer.hpp>

#include <shader.hpp>
#include <camera.hpp>
#include <UBO.hpp>

#include <vector>
#include <iostream> // For std::cerr

// Include your OpenGL wrapper classes and the STL loader
#include <VAO.hpp>
#include <VBO.hpp>
#include <EBO.hpp>
#include <stlImport.hpp> // Include the header for loading STL files
#include <paths.hpp> // Assuming projectPath() is defined here
#include <model.hpp> // NEW: Include the new Model class header

Camera* currentCamera;

// Global pointer for the loaded model instance
Model* mainModelInstance = nullptr; // Renamed from pyramidModelInstance

// Function to initialize the model data and OpenGL buffers for the main model
// Now responsible for creating the Model instance
void setupModels() {
    // Create an instance of your Model class
    // It will load the STL file and set up its OpenGL buffers in its constructor
    mainModelInstance = new Model(projectPath("res/models/pyramid.stl"), glm::vec3(1.0f, 0.0f, 0.0f)); // Red by default
}

// Function to clean up all dynamically allocated model resources
// Now responsible for deleting the Model instance
void cleanupModels() {
    if (mainModelInstance) {
        delete mainModelInstance;
        mainModelInstance = nullptr;
    }
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Only attempt to render if the model instance has been successfully created
    if (mainModelInstance) {
        // Define a model matrix to position and orient the model in the scene
        glm::mat4 modelMatrix = glm::mat4(1.0f); // Start with an identity matrix

        // Apply rotation to correct orientation (e.g., if model is Z-up and OpenGL is Y-up)
        modelMatrix = glm::rotate(modelMatrix, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate around X-axis

        // Translate the model back so it's visible in front of the camera
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, -2.0f));

        // Define light and camera properties (these can be moved to uniforms in a UBO later)
        glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);       // White light
        glm::vec3 lightPosition = glm::vec3(1.0f, 1.0f, 1.0f);    // Example light position
        glm::vec3 cameraPosition = currentCamera->position;       // Get camera position from your Camera class

        // Call the draw method of the Model instance
        mainModelInstance->draw(mainShader, modelMatrix, lightColor, lightPosition, cameraPosition);
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

