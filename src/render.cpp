#include <config.hpp>
#include <globals.hpp>

#include <Timer.hpp>

#include <shader.hpp>
#include <camera.hpp>

#include <lightObject.hpp>
#include <object.hpp>
#include <vector>

#define MAX_AMOUNT_LIGHTS 4

Camera* currentCamera;

// setting up objects
object* pyramid;

struct Light {
    glm::vec3 position;
    glm::vec4 color;
    GLfloat intensity;
};


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


LightObject* light;
LightObject* redLight;

void renderSetup() {

    pyramid = new object();
    light = new LightObject(lightShader, lightVertices, size(lightVertices), lightIndices, size(lightIndices), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), glm::vec3(0.8f, 0.5f, 0.5f), 1.5f);
    redLight = new LightObject(lightShader, lightVertices, size(lightVertices), lightIndices, size(lightIndices), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), glm::vec3(0.8f, 0.5f, -0.5f), 1.5f);

    vector<Light> lights;

    // send info about the light source to the object shader; can be set once if only one light present
    mainShader->activate();

    lights.push_back((Light){light->lightPosition, light->lightColor, light->lightIntensity});
    lights.push_back((Light){redLight->lightPosition, redLight->lightColor, redLight->lightIntensity});
    
    mainShader->setUniform("lightCount", (GLint)lights.size());

    // sends all the uniforms at once (setup) individual change might be required upon the light changing position
    for (int i = 0; i < lights.size() || i < MAX_AMOUNT_LIGHTS; ++i) {
        std::string prefix = "lights[" + std::to_string(i) + "]"; // Lights[0] for this case
        
        // Setting light position (vec3)
        mainShader->setUniform((prefix + ".position").c_str(), lights[i].position, true);
        
        // Setting light color (vec4)
        mainShader->setUniform((prefix + ".color").c_str(), lights[i].color, true);
        
        // Setting light intensity (float)
        mainShader->setUniform((prefix + ".intensity").c_str(), lights[i].intensity, true);
    }

}

void render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // sends draw matrices to the default buffer
    currentCamera->updateProjection(mainShader);

    // needs to be sent every frame - sends camera position to shader for specular lighting
    mainShader->setUniform("cameraPosition", currentCamera->position);

    pyramid->draw();

    // sends draw matrices to the light buffer
    currentCamera->updateProjection(lightShader);

    light->updatePosition();
    light->applyLightColor();
    light->draw();

    redLight->updatePosition();
    redLight->applyLightColor();
    redLight->draw();

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