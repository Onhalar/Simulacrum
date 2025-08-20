#include <config.hpp>
#include <types.hpp>
#include <simObject.hpp>
#include <scenes.hpp>
#include <debug.hpp>
#include <FormatConsole.hpp>

#include <UBO.hpp>
#include <shader.hpp>
#include <camera.hpp>
#include <renderDefinitions.hpp>

void setupShaderMetrices(Shader* shader);

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