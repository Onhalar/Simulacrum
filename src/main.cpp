#include <config.hpp>
#include <filesystem>
#include <globals.hpp>
#include <state.hpp>

#include <simObject.hpp>
#include <customMath.hpp>
#include <physicsThread.hpp>
#include <renderDefinitions.hpp>
#include <string>

#include <imgui.h>

#include "render.cpp" // provides render as well as GUI
#include "settings.cpp"
#include "simulation.cpp"
#include "input.cpp"

#include "setup/simSetup.cpp"
#include "setup/renderSetup.cpp"

void createWindow();
void setupOpenGL();
void mainLoop();

void setupShaders();
void setupModels();

void setupPostProcess();

void setupSimulation();

void setupGui();
void GuiCameraInterruption();

void cleanup();

int main(int argc, char **argv) {
    mainState = state::starting;

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

    // Setup ImGui after OpenGL context is ready
    setupGui();

    // Call setupModels() to load the STL file and prepare its OpenGL buffers
    // This must be called after OpenGL context is created and GLAD is loaded.
    setupModels();

    setupPostProcess();

    setupSimulation();

    transitionState(state::paused); // here so that the physics thread can be started but scene does not have to be loaded yet

    mainLoop();

    // Call cleanup() to free all allocated model resources before exiting
    mainState = state::stopping;
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
    glClearColor(backgroundColor.decR , backgroundColor.decG, backgroundColor.decB, backgroundColor.a);

    setupShaders();

    glfwSwapInterval(VSync);
}

void setupGui() {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = windowRounding;    // Window corners
    style.FrameRounding = frameRounding;     // Buttons, sliders, etc.

    // Making Fonts
    io.Fonts->Clear();
    ImFontConfig config;

    std::string fontPath = projectPath(resourcePath / fontFile);

    if (std::filesystem::exists((std::filesystem::path)fontPath)) {
        config.OversampleH = 2;
        config.OversampleV = 2;

        Fonts["large"] = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), fontSize * 1.1f, &config);
        Fonts["larger"] = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), fontSize * 1.35f, &config);
        Fonts["largest"] = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), fontSize * 1.5f, &config);
        Fonts["normal"] = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), fontSize, &config);
    }
    else {
        // large
        config.SizePixels = fontSize * 1.1f;
        Fonts["large"] = io.Fonts->AddFontDefault(&config);
            
        //larger
        config.SizePixels = fontSize * 1.35f;
        Fonts["larger"] = io.Fonts->AddFontDefault(&config);

        //largest
        config.SizePixels = fontSize * 1.5f;
        Fonts["largest"] = io.Fonts->AddFontDefault(&config);

        // default
        config.SizePixels = fontSize;
        Fonts["normal"] = io.Fonts->AddFontDefault(&config);
    }

    // Enable keyboard and gamepad controls (optional)
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(mainWindow, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void mainLoop() {
    TinyInt frameCount = 0;

    if (!isPowerOfTwo(lightUpdateFrameSkip)) {
        lightUpdateFrameSkip = roundUpToPowerOfTwo(lightUpdateFrameSkip);
    }
    updateLightSources(); // inital calculation (positions) + sending data to shaders

    physicsThread = std::thread(physicsThreadFunction);

    while (!glfwWindowShouldClose(mainWindow)) {
        auto frameStart = steady_clock::now(); // Use std::chrono
        if (showScenePicker) { supressCameraControls = true; } // don't use cameara when switching scene
        else { supressCameraControls = false; }

        // handles events such as resizing and creating window
        glfwPollEvents();

        if (!isMinimized) { // Custom Actions

            // ----==[ RENDERING ]==----

            // WINDOW INTERACTIONS
            if (glfwGetWindowAttrib(mainWindow, GLFW_FOCUSED)) {
                GuiCameraInterruption();

                handleInputs();
                
                currentCamera->updateCameraValues(renderDistance, cameraSensitivity, cameraSpeed, fovDeg);
                currentCamera->handleInputs(mainWindow);
                for(auto shader: Shaders) {
                    currentCamera->updateProjection(shader.second);
                }
            }

            // y       = 8   = 1000
            // y - 1   = 7   = 0111
            if ( (frameCount & (lightUpdateFrameSkip - 1)) == 0 ) {
                updateLightSources();
            }

            render();
        }

        static steady_clock::time_point lastTime;

        // here just so everything doesn't fly 10 000 km off the screen
        static bool isFirstFrame = true;
        if (isFirstFrame) {
            lastTime = frameStart;
            isFirstFrame = false;
        }

        auto frameEnd = steady_clock::now();
        auto elapsed = duration_cast<nanoseconds>(frameEnd - frameStart);

        if (elapsed < frameDuration && !VSync) {
            std::this_thread::sleep_for((frameDuration - elapsed) * staticDelayFraction);

            // spin delay for frames
            if (staticDelayFraction < 1.0f) {
                while (true)
                {
                    std::this_thread::sleep_for(spinDelay);
                    if (steady_clock::now() >= frameStart + frameDuration) { break; }
                }
            }
        }

        ++frameCount; // does not need to be checked, unsigned types wrap around.

        frameEnd = steady_clock::now();

        deltaTime = duration_cast<nanoseconds>(frameEnd - lastTime).count() / 1'000'000'000.0;
            
        lastTime = frameEnd;
    }
}

void cleanup() {
    // must be called before shutting down the main window
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    //ImGUI will destroy its fonts by itself - ImGui::DestroyContext();
    Fonts.clear();

    physicsRunning = false; 
    if (physicsThread.joinable()) { physicsThread.join(); }

    delete currentCamera;
    currentCamera = nullptr;

    if (lightBlockUBO) {
        delete lightBlockUBO;
        lightBlockUBO = nullptr;
    }

    for (auto& [key, lightObject] : lightQue) {
        delete lightObject;
        lightObject = nullptr;
    }
    lightQue.clear();

    for (auto& [key, scene] : Scenes::allScenes) {
        delete scene;
    }
    Scenes::allScenes.clear();
    Scenes::currentScene = nullptr;

    for (const auto& FBO : FBOs) { delete FBO.second; }
    FBOs.clear();

    for (const auto& LightObject : lightQue) { delete LightObject.second; }
    lightQue.clear();

    for (const auto& simObject : SimObjects) { delete simObject.second; }
    SimObjects.clear();

    for (const auto& model : Models) { delete model.second; }
    Models.clear();

    for(const auto& shader : Shaders) { delete shader.second; } // destroys class on heap and clears OpenGl binaries
    Shaders.clear(); // remoces map entries if classes were not cleared before -> dangling pointers
}
