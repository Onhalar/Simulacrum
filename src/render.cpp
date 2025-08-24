#include <config.hpp>
#include <globals.hpp>

//#include <Timer.hpp>

#include <shader.hpp>
#include <camera.hpp>

#include <paths.hpp> // Assuming projectPath() is defined here
#include <model.hpp> // Include the Model class header
#include <lightObject.hpp>
#include <simObject.hpp>

#include <FBO.hpp>

#include <scenes.hpp>
#include <customMath.hpp>
#include <renderDefinitions.hpp>

void render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    static FBO* postProcessFBO;
    static Shader* postProcessShader;

    if (doPostProcess) {
        postProcessFBO = FBOs["postProcess"];
        postProcessShader = Shaders["postProcess"];
    }

    // Only attempt to render if the model instance has been successfully created

    if (Scenes::currentScene.empty()) {
        if (debugMode) { std::cout << formatError("ERROR") << ": current scene list emtpty... skipping frame." << std::endl; }
        return;
    }

    if (doPostProcess) {
        postProcessFBO->bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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

    if (doPostProcess) {
        postProcessFBO->unbind();

        postProcessFBO->draw(postProcessShader);
    }

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


void resize(GLFWwindow *window, int width, int height) {
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
