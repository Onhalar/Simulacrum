#include <config.hpp>
#include <globals.hpp>

#include <shader.hpp>
#include <camera.hpp>

#include <paths.hpp> 
#include <model.hpp>
#include <lightObject.hpp>
#include <simObject.hpp>

#include <FBO.hpp>

#include <scenes.hpp>
#include <customMath.hpp>
#include <renderDefinitions.hpp>

#include <physicsThread.hpp>

#include "glm/fwd.hpp"
#include "gui.cpp"
#include "state.hpp"

void renderGui(); // function in gui.cpp

void render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (Scenes::currentScene) {
        static FBO* postProcessFBO;
        static Shader* postProcessShader;

        if (doPostProcess) {
            postProcessFBO = FBOs["postProcess"];
            postProcessShader = Shaders["postProcess"];
        }

        // Only attempt to render if the model instance has been successfully created

        if (Scenes::currentScene->objects.empty()) {
            if (debugMode) { std::cout << formatError("ERROR") << ": current scene list emtpty... skipping frame." << std::endl; }
            return;
        }

        if (doPostProcess) {
            postProcessFBO->bind();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }

        for (const auto& simObject : Scenes::currentScene->objects) {
            Shader* shader = simObject->shader;
            Model* model = simObject->model;


            glm::dvec3 renderPos;
            {
                std::lock_guard<std::mutex> lock(physicsMutex);
                renderPos = simObject->vertPosition; // copy safe position
            }

            shader->activate();

            if (simulateObjectRotation && mainState != state::paused) {
                simObject->modelMatrix = glm::rotate(simObject->modelMatrix, (float)(glm::radians(simObject->vertexRotation) * simulationSpeed * deltaTime), glm::vec3(0,1,0)); // temporarily rotate around Z axii
                shader->applyModelMatrix( calcuculateModelMatrixFromPosition(renderPos) * simObject->modelMatrix /*rotation*/ * (simObject->model->isDerived ? simObject->model->transform : 1.0f) /*scaling*/ );
            }
            else if (!simObject->model->isDerived) {
                shader->applyModelMatrix(calcuculateModelMatrixFromPosition(renderPos) * (simObject->model->isDerived ? simObject->model->transform : 1.0f));
            }

            simObject->draw( true /*skip sending derived model matrix*/);
        }

        if (doPostProcess) {
            postProcessFBO->unbind();

            postProcessFBO->draw(postProcessShader);
        }
    }

    renderGui();

    glfwSwapBuffers(mainWindow);
}

void setupPostProcess() {
    if (!doPostProcess || Shaders.find("postProcess") == Shaders.end()) {
        doPostProcess = false;
        return;
    }

    int width, height;
    glfwGetFramebufferSize(mainWindow, &width, &height);

    FBOs["postProcess"] = new FBO(width, height);
    Shaders["postProcess"]->setUniform("resolution", glm::vec2(width, height));

    Shaders["postProcess"]->setUniform("enableFXAA", doFXAA);
    Shaders["postProcess"]->setUniform("inverseColors", inverseColors);

}

void updateLightSourcePositions() {
    for (const auto& light : lightQue) {
        if (SimObjects.find(light.first) != SimObjects.end()) {
            light.second->position = SimObjects[light.first]->vertPosition;
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


void resize(GLFWwindow *window, int width, int height) {
    if (fullscreen) { return; } // here just in case so that values won't change after entering fullscreen.
    if ((width | height) == 0) {
        isMinimized = true;
        return;
    }
    else { isMinimized = false; }

    windowWidth = width;
    windowHeight = height;

    // Set the viewport first
    glViewport(0, 0, width, height);
    
    // Ensure the mainShader is active before updating projection
    for (const auto& shaderPair : Shaders) {
        auto shader = shaderPair.second;
        if (shader) {
            shader->activate();
            currentCamera->updateProjection(width, height, shader);
        }
    }

    for (const auto& FBO : FBOs) {
        FBO.second->resize(width, height);
    }

    if (doPostProcess) {
        Shaders["postProcess"]->setUniform("resolution", glm::vec2(width, height));
    }
}

// FULLSCREEN

inline glm::ivec2 oldSize(windowWidth, windowHeight);
inline glm::ivec2 oldPosition(0);

void enterFullscreen() {
    if (fullscreen) { return; }

    oldSize = glm::ivec2(windowWidth, windowHeight);

    
    glfwGetWindowPos(mainWindow, &oldPosition.x, &oldPosition.y);
    glfwGetWindowSize(mainWindow, &oldSize.x, &oldSize.y);
    
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    glfwSetWindowMonitor(mainWindow, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);

    resize(mainWindow, mode->width, mode->height);

    fullscreen = true;
}


void exitFullscreen() {
    if (!fullscreen) { return; }
    
    fullscreen = false;
    
    // Restore windowed mode with saved position and size
    glfwSetWindowMonitor(mainWindow, nullptr, oldPosition.x, oldPosition.y, oldSize.x, oldSize.y, GLFW_DONT_CARE);
    
    // Force a window refresh on Windows because of course I have to ...
    #ifdef _WIN32
    glfwSetWindowPos(mainWindow, oldPosition.x, oldPosition.y);
    glfwSetWindowSize(mainWindow, oldSize.x, oldSize.y);
    #endif
    
    resize(mainWindow, oldSize.x, oldSize.y);
}