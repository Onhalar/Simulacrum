#include <config.hpp>
#include <globals.hpp>

#include "render.cpp"

void createWindow();
void setupOpenGL();
void mainLoop();

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

    setupOpenGL();

    renderSetup();

    mainLoop();

    return 0;
}



void createWindow() {
    GLFWwindow* window;
    GLFWimage icon;
    
    // tries to initialize GLFW, if fails => error & exit
    if (!glfwInit()) {
        cerr << formatError("ERROR") << ": Could not initialize GLFW" << endl;
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
    
    // tries to load in the icon
    icon.pixels = stbi_load(projectPath(iconPath).c_str(), &icon.width, &icon.height, 0, 4);

    // checks whther the icon is loaded successfully
    // if yes => sets icon and clears it from memory
    // if no  => prints an error
    if (icon.pixels) {
        glfwSetWindowIcon(window, 1, &icon);
        stbi_image_free(icon.pixels);
    }
    else {
        cerr << formatError("ERROR") << ": Could not open icon '" << formatPath(iconPath) << "'" << endl;
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
    }


    int width, height;

    // retrieves the size of GLFW window
    glfwGetFramebufferSize(mainWindow, &width, &height);

    // sets OpenGL viewport (plane onto which will be deawn)
    glViewport(0, 0, width, height);

    // sets background color defined in header
    apply(glClearColor, defaultBackgroundColor);

    mainShader = new Shader(
        projectPath("shaders/default.vert"),
        projectPath("shaders/default.frag")
    );

    lightShader = new Shader(
        projectPath("shaders/light.vert"),
        projectPath("shaders/light.frag")
    );

    setupShaderMetrices(mainShader);

    setupShaderMetrices(lightShader);
}


void mainLoop() {
    renderCycle(); // initializes rendering loop

    while (!glfwWindowShouldClose(mainWindow)) {

        // handles scheduled tasks
        handleSchedule();

        // handles events such as resizing and creating window
        glfwPollEvents(); 
    }

    glfwDestroyWindow(mainWindow);
    glfwTerminate();
}