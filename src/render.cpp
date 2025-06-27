#include <config.hpp>
#include <globals.hpp>

#include <Timer.hpp>

#include <shader.hpp>
#include <camera.hpp>
#include <UBO.hpp>

#include <lightObject.hpp>
#include <object.hpp>
#include <vector>

#define MAX_AMOUNT_LIGHTS 4

Camera* currentCamera;

// setting up objects
object* pyramid;

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

struct Light {
    glm::vec3 position = glm::vec3(0.0f);
    float _pad1 = 0.0f;

    glm::vec4 color = glm::vec4(1.0f);
    float intensity = 1.0f;
    float falloff = 1.0f;  // Added falloff
    float _pad2[2] = {0.0f, 0.0f}; // Adjust padding to maintain alignment

    // Default constructor
    Light() = default;

    // Custom constructor
    Light(glm::vec3 pos, glm::vec4 col, float inten, float off) {
        position = pos; color = col; intensity = inten; falloff = off;
    }
};

// Define LightBlock struct that represents the UBO structure
struct LightBlock {
    Light lights[MAX_AMOUNT_LIGHTS];
    alignas(16) int lightCount;  // Align to 16 bytes
    int _pad[3] = {0, 0, 0};     // Padding to match std140
};

vector<Light> lights;
UBO* lightUBO;
LightBlock lightBlock;

void renderSetup() {

    pyramid = new object();
    light = new LightObject(lightShader, lightVertices, size(lightVertices), lightIndices, size(lightIndices), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), glm::vec3(0.8f, 0.5f, 0.5f), 0.2f, 1.5f);
    redLight = new LightObject(lightShader, lightVertices, size(lightVertices), lightIndices, size(lightIndices), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), glm::vec3(0.8f, 0.5f, -0.5f), 0.2f, 1.5f);


    // send info about the light source to the object shader; can be set once if only one light present
    mainShader->activate();

    lights.push_back((Light){light->lightPosition, light->lightColor, light->lightIntensity, light->lightDistance});
    lights.push_back((Light){redLight->lightPosition, redLight->lightColor, redLight->lightIntensity, light->lightDistance});
    
    lightBlock.lightCount = lights.size();

    // Copy data from the vector to the LightBlock
    for (int i = 0; i < lightBlock.lightCount; ++i) {
        lightBlock.lights[i] = lights[i];
    }

    // Create a UBO to store light data
    lightUBO = new UBO(sizeof(LightBlock), &lightBlock, GL_DYNAMIC_DRAW);

    // Bind the UBO to a binding point (e.g., binding 0)
    lightUBO->bind();

}

void updateLights() {
    lights.clear();

    lights.push_back((Light){light->lightPosition, light->lightColor, light->lightIntensity, light->lightDistance});
    lights.push_back((Light){redLight->lightPosition, redLight->lightColor, redLight->lightIntensity, light->lightDistance});
    
    lightBlock.lightCount = lights.size();

    // Copy data from the vector to the LightBlock
    for (int i = 0; i < lightBlock.lightCount; ++i) {
        lightBlock.lights[i] = lights[i];
    }

    lightUBO->update(0, sizeof(lightBlock), &lightBlock);
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

    static bool switchDir = false;
    if (redLight->lightPosition.z >= 0.35f || redLight->lightPosition.z <= -0.5f) { switchDir = !switchDir; }
    redLight->updatePosition(redLight->lightPosition + glm::vec3(0.0f, 0.0f, -0.3f  * deltaTime * (switchDir ? 1.0f : -1.0f)));

    updateLights();

    light->updatePosition();
    lightShader->modelMatrix = glm::scale(lightShader->modelMatrix, glm::vec3(0.5f, 0.5f, 0.5f));
    lightShader->applyModelMatrix(); // can do it this ways since light->updatePosition() resets the shader model matrix each time.
    light->applyLightColor();
    light->draw();

    redLight->updatePosition();
    lightShader->modelMatrix = glm::scale(lightShader->modelMatrix, glm::vec3(0.5f, 0.5f, 0.5f));
    lightShader->applyModelMatrix();
    redLight->applyLightColor();
    redLight->draw();

    glfwSwapBuffers(mainWindow);
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