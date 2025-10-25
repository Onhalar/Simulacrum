#include <config.hpp>
#include <debug.hpp>
#include <globals.hpp>
#include <renderDefinitions.hpp>
#include <physicsThread.hpp>

// 3rd party headers
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <glm/geometric.hpp>

// use to display the current scene
#include <scenes.hpp>

#include <map>
#include <chrono>


std::map<std::string, ImFont*> Fonts = {};

ImGuiIO *io;


void renderSceneGraph();
void renderSimSpeedDisplay();
void renderSettingsMenu();


void renderGui() {
    io = &ImGui::GetIO();

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Render setup
    renderSimSpeedDisplay();
    renderSceneGraph();

    renderSettingsMenu();

    // Rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}



void renderSettingsMenu() {
    static bool showMenu = false;
    static bool wasPressed = false;
    
    // Detect single key press (not held)
    bool isPressed = glfwGetKey(mainWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS;
    if (isPressed && !wasPressed) {
        showMenu = !showMenu;
    }
    wasPressed = isPressed | currentCamera->focused;

    if (!showMenu) {
        ImGui::SetNextWindowBgAlpha(0.35f);
        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always, ImVec2(0.0f, 0.0f));

        ImGui::Begin("SettingsMenuButton", nullptr, 
                ImGuiWindowFlags_NoTitleBar | 
                ImGuiWindowFlags_NoResize | 
                ImGuiWindowFlags_AlwaysAutoResize |
                ImGuiWindowFlags_NoMove);

        static ImVec4 transparent(0.0f, 0.0f, 0.0f, 0.0f);

        ImGui::PushStyleColor(ImGuiCol_Button, transparent);

        if (ImGui::Button("Menu")) { showMenu = true; }

        ImGui::PopStyleColor();

        ImGui::End();
    }

    // Only render if settings should be shown
    if (!showMenu || currentCamera->focused) { showMenu = false; return; }

    // Center the window
    ImGui::SetNextWindowPos(ImVec2(io->DisplaySize.x * 0.5f, io->DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(io->DisplaySize.x * 0.9f, io->DisplaySize.y * 0.9f), ImGuiCond_Always);
    
    ImGui::PushFont(Fonts["larger"]);
    ImGui::Begin("Menu", &showMenu, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
    ImGui::PopFont();

    // OPTIONS

    ImGui::PushFont(Fonts["largest"]);
    ImGui::Text("Options");
    ImGui::PopFont();

    ImGui::Spacing();

    ImGui::PushFont(Fonts["larger"]);

    if (ImGui::Button("Load Scene")) {
        // ToDo: Add scene Switching code here
    }

    ImGui::SameLine(0.0f, 10.0f);
    if (ImGui::Button("Quit")) {
        glfwSetWindowShouldClose(mainWindow, true);
    }

    ImGui::PopFont();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // SETTINGS

    ImGui::PushFont(Fonts["largest"]);
    ImGui::Text("Settings");
    ImGui::PopFont();

    ImGui::Spacing();

    ImGui::PushFont(Fonts["large"]);

    if (ImGui::CollapsingHeader("Graphics", ImGuiTreeNodeFlags_DefaultOpen)) {

        ImGui::Checkbox("VSync", (bool*)&VSync);
        if (!VSync) {
            static int lastMaxFPS = maxFrameRate;

            ImGui::SliderInt("Max FPS", &maxFrameRate, 10, 300, "%i FPS");

            if (lastMaxFPS != maxFrameRate) {
                frameDuration = nanoseconds(1'000'000'000 / maxFrameRate);
                lastMaxFPS = maxFrameRate;
            }
        }

        ImGui::Checkbox("Post Process", &doPostProcess);

        if (doPostProcess) {
            // FixMe: This is kinda a hack, consider rewriting
            static bool doFXAALocal = doFXAA, inverseColorsLocal = inverseColors;

            ImGui::Checkbox("FXAA", &doFXAALocal);
            ImGui::Checkbox("Inverse colors", &inverseColorsLocal);

            if (doFXAALocal != doFXAA && Shaders.find("postProcess") != Shaders.end()) {
                doFXAA = doFXAALocal;
                Shaders["postProcess"]->activate();
                Shaders["postProcess"]->setUniform("enableFXAA", doFXAA);
            }

            if (inverseColorsLocal != inverseColors && Shaders.find("postProcess") != Shaders.end()) {
                inverseColors = inverseColorsLocal;
                Shaders["postProcess"]->activate();
                Shaders["postProcess"]->setUniform("inverseColors", inverseColors);
            }
        }

        ImGui::SliderFloat("Render Distance (Vertex distance)", &renderDistance, 100.0f, 50'000.0f, "%.0f Vd");
    }
    
    if (ImGui::CollapsingHeader("Simulation", ImGuiTreeNodeFlags_DefaultOpen)) {

        static double min = 1.0, max = 5.0e5, simulationSpeedLocal = simulationSpeed;

        ImGui::SliderScalar("Simulation Speed", ImGuiDataType_Double, &simulationSpeedLocal, &min, &max, "%.2fx");

        if (simulationSpeedLocal != simulationSpeed) {
            std::lock_guard<std::mutex> lock(physicsMutex);
            simulationSpeed = simulationSpeedLocal;
        }


        static unsigned int phyiscsSubstepsLocal = phyiscsSubsteps;

        ImGui::SliderInt("Physics SubSteps", (int*)&phyiscsSubstepsLocal, 1, 32);

        if (phyiscsSubstepsLocal != phyiscsSubsteps) {
            std::lock_guard<std::mutex> lock(physicsMutex);
            phyiscsSubsteps = phyiscsSubstepsLocal;
        }

        
        ImGui::Checkbox("Simulate object rotation", &simulateObjectRotation);
        ImGui::Text("Physics FPS: %.0f", physicsSteps);
    }
    
    if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {

        ImGui::SliderFloat("FOV", &fovDeg, 30.0f, 120.0f, "%.2f Deg");
        ImGui::SliderFloat("Sensitivity", &cameraSensitivity, 50.0f, 300.0f, "%.0f");
        ImGui::SliderFloat("Camera Speed", &cameraSpeed, 1.0f, 100.0f, "%.0f");
    }
    
    ImGui::PopFont();

    ImGui::End();
}


void renderSimSpeedDisplay() {
    ImGui::SetNextWindowPos(ImVec2(io->DisplaySize.x * 0.5f, 10), ImGuiCond_Always, ImVec2(0.5f, 0.0f));

    ImGui::SetNextWindowBgAlpha(0.35f);
    ImGui::Begin("simspeedDisplay", nullptr, 
                ImGuiWindowFlags_NoTitleBar | 
                ImGuiWindowFlags_NoResize | 
                ImGuiWindowFlags_AlwaysAutoResize |
                ImGuiWindowFlags_NoMove);
        
    ImGui::PushFont(Fonts["larger"]);
    ImGui::Text("%ix", (int)simulationSpeed);
    ImGui::PopFont();

    ImGui::End();
}


void renderSceneGraph() {
    // Get the viewport size to position in top right
    ImGui::SetNextWindowPos(ImVec2(io->DisplaySize.x - 10, 10), ImGuiCond_Always, ImVec2(1.0f, 0.0f));

    ImGui::SetNextWindowBgAlpha(0.35f); // Semi-transparent so you can see through it
    ImGui::Begin("sceneGraph", nullptr, 
                ImGuiWindowFlags_NoTitleBar | 
                ImGuiWindowFlags_NoResize | 
                ImGuiWindowFlags_AlwaysAutoResize |
                ImGuiWindowFlags_NoMove);
    

    if (ImGui::TreeNode(Scenes::currentSceneID.c_str())) {
        for (const auto& object :Scenes::currentScene->objects) {
            if (ImGui::TreeNode(object->name.c_str())) {

                ImGui::BulletText("Mass: %g t", (double)object->mass);
                ImGui::BulletText("Radius: %.0f km", (double)object->radius);
                ImGui::BulletText("Velocity: %.2f km/s", (double)glm::length(object->realVelocity));
                ImGui::TreePop();
            }
        }
        ImGui::TreePop();
    }

    ImGui::End();
}

// checks if the mouse is above a ui element if so, disables camera controls for the frame
void GuiCameraInterruption() {
    if (ImGui::IsAnyItemActive() || ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow)) {
        supressCameraControls = true;
    }
} 