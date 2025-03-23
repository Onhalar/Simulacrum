#include <config.hpp>
#include <globals.hpp>
#include <Timer.hpp>
#include <shader.hpp>

#include "TEMP/dummyObject.hpp"

void render() {
    static dummyObject* object = new dummyObject();

    glClear(GL_COLOR_BUFFER_BIT);

    mainShader->activate();
    object->draw();

    glfwSwapBuffers(mainWindow);
}

void renderCycle() {
    render();

    static float lastTime = 0.0f;
    float currentTime = static_cast<float>(glfwGetTime());
    deltaTime = currentTime - lastTime;
    lastTime = currentTime;
    
    Timer(1000 / maxFrameRate, renderCycle);
}
