#include <config.hpp>
#include <globals.hpp>

#include <Timer.hpp>

#include <shader.hpp>
#include <camera.hpp>

#include <lightObject.hpp>
#include <object.hpp>

Camera* currentCamera;

void render() {
    static object* pyramid = new object();

    GLfloat lightVertices[] =
    { //     COORDINATES     //
        -0.1f, -0.1f,  0.1f,
        -0.1f, -0.1f, -0.1f,
         0.1f, -0.1f, -0.1f,
         0.1f, -0.1f,  0.1f,
        -0.1f,  0.1f,  0.1f,
        -0.1f,  0.1f, -0.1f,
         0.1f,  0.1f, -0.1f,
         0.1f,  0.1f,  0.1f
    };
    
    GLuint lightIndices[] =
    {
        1, 2, 0, // bottom
        2, 3, 0,
        7, 4, 0, // front
        3, 7, 0,
        3, 6, 7, // right
        3, 2, 6,
        2, 5, 6, // back
        2, 1, 5,
        1, 4, 5, // left
        1, 0, 4,
        4, 7, 5, // top
        7, 6, 5
    };

    static LightObject* light = new LightObject(lightShader, lightVertices, size(lightVertices), lightIndices, size(lightIndices));

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    currentCamera->updateProjection(mainShader);

    pyramid->draw();

    currentCamera->updateProjection(lightShader);

    light->draw();

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

void APIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
    GLsizei length, const GLchar *message, const void *userParam) {
std::cout << formatError("OpenGL Debug") << ": " << colorText(message, ANSII_YELLOW) << std::endl;
}