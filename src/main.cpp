#include <config.hpp>
#include <globals.hpp>
#include <iostream>

#include <simObject.hpp>

#include "render.cpp"
#include "settings.cpp"
#include "simulation.cpp"

// ***************************************
// ** ToDo: Add GUI with Dear ImGui   **
// ****************************************

void createWindow();
void setupOpenGL();
void mainLoop();

void setupShaders();
void renderSetup();
void setupModels();
void setupSimObjects();

void loadSettings(std::filesystem::path path);
void loadSimObjects(std::filesystem::path path);

void cleanupRender();
void cleanup();

int main(int argc, char **argv) {

    // attemps to extract current file location from call args
    if (filesystem::exists(argv[0])) {
        projectDir = ((filesystem::path)argv[0]).parent_path().parent_path();
    }

    // if fails, tries to get current working directory and hope it's correct
    else {
        projectDir = filesystem::current_path().parent_path();
    }

    createWindow();

    glfwSetFramebufferSizeCallback(mainWindow, resize);

    loadSettings(projectPath(settingsPath));

    setupOpenGL();

    // Call setupModels() to load the STL file and prepare its OpenGL buffers
    // This must be called after OpenGL context is created and GLAD is loaded.
    setupModels();
    renderSetup();

    loadSimObjects(projectPath(simObjectsConfigPath));
    setupSimObjects();

    mainLoop();

    // Call cleanup() to free all allocated model resources before exiting
    cleanupRender();
    cleanup();

    glfwDestroyWindow(mainWindow);
    glfwTerminate();

    return 0;
}



void createWindow() {
    GLFWwindow* window;
    GLFWimage icon;
    
    // tries to initialize GLFW, if fails => error & exit
    if (!glfwInit()) {
        std::cerr << formatError("ERROR") << ": Could not initialize GLFW" << std::endl;
        exit(-1);
    }

    // specifying which OpenGL version to use
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    if (debugMode) {
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    }

    // creating GLFW window
    window = glfwCreateWindow(defaultWindowWidth, defaultWindowHeight, windowName, NULL, NULL);

    // sets the minimum and maximum window size
    glfwSetWindowSizeLimits(window, minWindowWidth, minWindowHeight, GLFW_DONT_CARE, GLFW_DONT_CARE);
    
    icon.pixels = stbi_load(projectPath(iconPath).c_str(), &icon.width, &icon.height, 0, 4);

    // checks whther the icon is loaded successfully
    // if yes => sets icon and clears it from memory
    // if no  => prints an error
    if (icon.pixels) {
        glfwSetWindowIcon(window, 1, &icon);
        stbi_image_free(icon.pixels);
    }
    else {
        std::cerr << formatError("ERROR") << ": Could not open icon '" << formatPath(iconPath) << "'" << std::endl;
    }

    // tell GLFW that the created window is the one to be used
    glfwMakeContextCurrent(window);

    // set global variable to the window for future manupulation
    mainWindow = window;
}

void setupOpenGL() {

    // loads in OpenGL functions from GLAD
    gladLoadGL();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS); // default but good to specify

    // Enable face culling (you already know this part)
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    if (debugMode) {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(MessageCallback, 0);

        std::cout << "\n" << formatRole("Info") << " " << glGetString(GL_VERSION) << "\n" << std::endl;
    }

    int width, height;

    // retrieves the size of GLFW window
    glfwGetFramebufferSize(mainWindow, &width, &height);

    // sets OpenGL viewport (plane onto which will be deawn)
    glViewport(0, 0, width, height);

    // sets background color defined in header
    // Assuming 'apply' is a macro or function that correctly calls glClearColor
    apply(glClearColor, defaultBackgroundColor);

    setupShaders();

    glfwSwapInterval(VSync);
}

void mainLoop() {

    while (!glfwWindowShouldClose(mainWindow)) {
        auto frameStart = steady_clock::now(); // Use std::chrono

        // handles events such as resizing and creating window
        glfwPollEvents();

        // handles scheduled tasks
        //handleSchedule();
        
        currentCamera->handleInputs(mainWindow);
        for(auto shader: Shaders) {
            currentCamera->updateProjection(shader.second);
        }

        render();

        static steady_clock::time_point lastTime; // Use std::chrono

        // here just so everything doesn't fly 10 000 km off the screen
        static bool isFirstFrame = true;
        if (isFirstFrame) {
            lastTime = frameStart;
            isFirstFrame = false;
        }

        auto frameEnd = steady_clock::now(); // Use std::chrono
        auto elapsed = duration_cast<nanoseconds>(frameEnd - frameStart); // Use std::chrono

        if (elapsed < frameDuration && !VSync) {
            std::this_thread::sleep_for((frameDuration - elapsed) * staticDelayFraction); // Use std::this_thread

            // spin delay for frames
            if (staticDelayFraction < 1.0f) {
                while (true)
                {
                    std::this_thread::sleep_for(spinDelay); // Use std::this_thread
                    if (steady_clock::now() >= frameStart + frameDuration) { break; } // Use std::chrono
                }
            }
        }

        frameEnd = steady_clock::now(); // Use std::chrono

        deltaTime = duration_cast<nanoseconds>(frameEnd - lastTime).count() / 1'000'000'000.0;
            
        lastTime = frameEnd;
    }
}

void cleanup() {
    for (const auto& simObject : SimObjects) { delete simObject.second; }
    SimObjects.clear();

    for (const auto& model : Models) { delete model.second; }
    Models.clear();

    for(const auto& shader : Shaders) { delete shader.second; } // destroys class on heap and clears OpenGl binaries
    Shaders.clear(); // remoces map entries if classes were not cleared before -> dangling pointers
}