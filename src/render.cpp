#include <config.hpp>
#include <globals.hpp>

#include <Timer.hpp>
#include <shader.hpp>
#include <camera.hpp>

#include "TEMP/dummyObject.hpp"

Camera* currentCamera;

void render() {
    static dummyObject* object = new dummyObject();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mainShader->activate();

    object->draw();

    glfwSwapBuffers(mainWindow);
}

void renderCycle() {
    currentCamera->handleInputs(mainWindow, mainShader);
    
    render();

    static float lastTime = 0.0f;
    float currentTime = static_cast<float>(glfwGetTime());
    deltaTime = currentTime - lastTime;
    lastTime = currentTime;
    
    Timer(1000 / maxFrameRate, renderCycle);
}

void resize(GLFWwindow *window, int width, int height) {
    mainShader->activate();
    
    currentCamera->updateProjection(width, height, mainShader);
}

void setupShaderMetrices(Shader* shader) {
    int windowWidth, windowHeight;
    glfwGetFramebufferSize(mainWindow, &windowWidth, &windowHeight);

    shader->activate();

    currentCamera = new Camera(windowWidth, windowHeight, glm::vec3(0.0f, 0.5f, 2.0f));

    currentCamera->updateProjection(windowWidth, windowHeight, shader);

    shader->applyModelMatrix();
}