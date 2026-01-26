#include <state.hpp>
#include <config.hpp>
#include <debug.hpp>
#include <globals.hpp>

#include <renderDefinitions.hpp>
#include <physicsThread.hpp>
#include <scenes.hpp>

// 3rd party headers
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <glm/geometric.hpp>

// use to display the current scene
#include <scenes.hpp>

#include <map>
#include <chrono>
#include <string>


std::map<std::string, ImFont*> Fonts = {};

ImGuiIO *io;


void renderSceneGraph();
void renderSimPerfDisplay();
void renderSettingsMenu();
void renderScenePicker();
void renderBackgChanger();

void renderGui() {
    io = &ImGui::GetIO();

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Render setup
    if (showScenePicker) {
        renderScenePicker();
    }
    else if (showBackgroundChanger) {
        renderBackgChanger();
    }
    else {
        renderSimPerfDisplay();
        renderSceneGraph();

        renderSettingsMenu();
    }

    // Rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}





void renderScenePicker() {
    transitionState(state::paused);

    ImGui::SetNextWindowPos(ImVec2(io->DisplaySize.x * 0.5f, io->DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(io->DisplaySize.x * 0.5f, io->DisplaySize.y * 0.5f), ImGuiCond_Always);
    
    ImGui::PushFont(Fonts["larger"]);
    ImGui::Begin("Scene Picker", &showScenePicker, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
    ImGui::PopFont();

    for (const auto& [name, scene] : Scenes::allScenes) {
        ImGui::TextUnformatted(name.c_str());

        std::string button_label = "Switch##" + name;

        ImGui::SameLine(0.0f, 10.0f);
        if (ImGui::Button(button_label.c_str())) {
            switchSceneAndCalculateObjects(name);

            showScenePicker = false;
            transitionState(state::running);

            if (showMenu) { showMenu = false; } // ensures clear first look
        }
    }

    ImGui::End();
}

void renderBackgChanger() {
    transitionState(state::paused);

    ImGui::SetNextWindowPos(ImVec2(io->DisplaySize.x * 0.5f, io->DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(io->DisplaySize.x * 0.5f, io->DisplaySize.y * 0.5f), ImGuiCond_Always);
    
    ImGui::PushFont(Fonts["larger"]);
    ImGui::Begin("Background changer", &showBackgroundChanger, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
    ImGui::PopFont();

    ImGui::SliderFloat("R", &backgroundColor.decR, 0.0f, 1.0f, "%.2f");
    ImGui::SliderFloat("G", &backgroundColor.decG, 0.0f, 1.0f, "%.2f");
    ImGui::SliderFloat("B", &backgroundColor.decB, 0.0f, 1.0f, "%.2f");

    glClearColor(backgroundColor.decR, backgroundColor.decG, backgroundColor.decB, backgroundColor.a);

    ImGui::End();
}

void renderSettingsMenu() {
    static bool wasPaused;

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

        if (ImGui::Button("Menu")) {
            showMenu = true;
            if (mainState == state::paused) { wasStatePausedBeforeMenu = true; }
        }

        ImGui::PopStyleColor();

        ImGui::End();

    }

    // Only render if settings should be shown
    if (!showMenu) { return; }
    if (currentCamera->focused) {
        showMenu = false;
        if (mainState == state::paused && !wasStatePausedBeforeMenu) { transitionState(state::running); }
        wasStatePausedBeforeMenu = false;
    }

    if (mainState != state::paused) { transitionState(state::paused); } // failsafe

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
        showScenePicker = true;
    }

    ImGui::SameLine(0.0f, 10.0f);
    if (ImGui::Button("Change Background")) {
        showBackgroundChanger = true;
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

        static bool fullscreenLocal = fullscreen;
        if (settingsUpdated) { fullscreenLocal = fullscreen; }

        ImGui::Checkbox("FullScreen", &fullscreenLocal);
        if (fullscreenLocal != fullscreen) {
            if (fullscreenLocal) { enterFullscreen(); } // fullscreen flag is managed by theese functions directly for maximum consistency
            else { exitFullscreen(); }
            fullscreenLocal = fullscreen;
        }

        ImGui::Checkbox("Render non-simulated objects", &renderUnsimulated);
        ImGui::Checkbox("Show FPS", &showFPS);
        ImGui::Checkbox("Show elapsed sim time", &showElapsedSimTime);

        static bool localVsync = VSync;
        ImGui::Checkbox("VSync", &localVsync);
        if (localVsync != VSync) {
            VSync = localVsync;
            glfwSwapInterval(VSync);
        }

        if (!VSync) {
            static int lastMaxFPS = maxFrameRate;

            ImGui::SliderInt("Max FPS", &lastMaxFPS, 10, 300, "%i FPS");

            if (lastMaxFPS != maxFrameRate) {
                lastMaxFPS = (lastMaxFPS / (int)10) * 10;

                frameDuration = nanoseconds(1'000'000'000 / maxFrameRate);
                maxFrameRate = lastMaxFPS;
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

        static float ambientStrengthLocal = ambientStrength;
        ImGui::SliderFloat("Ambient Light Strength", &ambientStrengthLocal, 0.0f, 1.0f, "%.2f");
        if (ambientStrengthLocal != ambientStrength) {
            for (const auto& [shaderID, shader] : Shaders) {
                ambientStrength = ambientStrengthLocal;
                shader->setUniform("ambientStrength", ambientStrength);
            }
        }

        // udpates every frame in main loop
        ImGui::SliderFloat("Render Distance (Vertex distance)", &renderDistance, 100.0f, 50'000.0f, "%.0f Vd");
    }
    
    if (ImGui::CollapsingHeader("Simulation", ImGuiTreeNodeFlags_DefaultOpen)) {

        static bool extraSlowSimSpeed = false;
        double min = extraSlowSimSpeed ? 1 : 2.5e4, max = extraSlowSimSpeed ? 5e3 : 1.0e7, simulationSpeedLocal = simulationSpeed;

        ImGui::SliderScalar("Simulation Speed", ImGuiDataType_Double, &simulationSpeedLocal, &min, &max, "%.0fx");

        if (simulationSpeedLocal != simulationSpeed) {
            std::lock_guard<std::mutex> lock(physicsMutex);
            simulationSpeed = simulationSpeedLocal;
        }

        ImGui::SameLine();
        ImGui::Checkbox("Extra slow", &extraSlowSimSpeed);

        static unsigned int phyiscsSubstepsLocal = phyiscsSubsteps;

        ImGui::SliderInt("Physics SubSteps", (int*)&phyiscsSubstepsLocal, 1, 512);

        if (phyiscsSubstepsLocal != phyiscsSubsteps) {
            std::lock_guard<std::mutex> lock(physicsMutex);
            phyiscsSubsteps = phyiscsSubstepsLocal;
        }

        ImGui::Checkbox("Cartoon Color Mode", &cartoonColorMode);
        ImGui::Checkbox("Simulate object rotation", &simulateObjectRotation);
        ImGui::Text("Physics FPS: %.0f", physicsSteps);
    }
    
    if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {

        ImGui::SliderFloat("FOV", &fovDeg, 30.0f, 120.0f, "%.1f Deg");
        ImGui::SliderFloat("Sensitivity", &cameraSensitivity, 50.0f, 300.0f, "%.0f");
        ImGui::SliderFloat("Camera Speed", &cameraSpeed, 1.0f, 250.0f, "%.0f");
    }
    
    ImGui::PopFont();

    ImGui::End();

    if (settingsUpdated) { settingsUpdated = false; }
}


void renderSimPerfDisplay() {
    ImGui::SetNextWindowPos(ImVec2(io->DisplaySize.x * 0.5f, 10), ImGuiCond_Always, ImVec2(0.5f, 0.0f));

    ImGui::PushFont(Fonts["normal"]);

    ImGui::SetNextWindowBgAlpha(0.35f);
    ImGui::Begin("simspeedDisplay", nullptr, 
                ImGuiWindowFlags_NoTitleBar | 
                ImGuiWindowFlags_NoResize | 
                ImGuiWindowFlags_AlwaysAutoResize |
                ImGuiWindowFlags_NoMove);
        
    ImGui::PushFont(Fonts["larger"]);

    if (mainState == state::paused) { ImGui::Text("P A U S E D"); }
    else { ImGui::Text("%ix", (int)simulationSpeed); }

    if (showElapsedSimTime && trackSimTime) {
        auto totalSecs = duration_cast<seconds>(elapsedSimTime);

        auto y = duration_cast<years>(totalSecs);
        totalSecs -= y;

        auto d = duration_cast<days>(totalSecs);
        totalSecs -= d;

        auto h = duration_cast<hours>(totalSecs);
        totalSecs -= h;

        auto m = duration_cast<minutes>(totalSecs);
        totalSecs -= m;

        ImGui::SameLine();
        ImGui::Text("| %li year(s) %3li day(s) %2li hour(s) %2li minute(s)", y.count(), d.count(), h.count(), m.count());
    }

    if (showFPS) {
        ImGui::SameLine();
        ImGui::Text("| %.0f FPS", currentFPS);
    }

    ImGui::PopFont();

    ImGui::PopFont();

    ImGui::End();
}


void renderSceneGraph() {
    // Get the viewport size to position in top right
    ImGui::SetNextWindowPos(ImVec2(io->DisplaySize.x - 10, 10), ImGuiCond_Always, ImVec2(1.0f, 0.0f));

    ImGui::PushFont(Fonts["normal"]);

    ImGui::SetNextWindowBgAlpha(0.35f); // Semi-transparent so you can see through it
    ImGui::Begin("sceneGraph", nullptr, 
                ImGuiWindowFlags_NoTitleBar | 
                ImGuiWindowFlags_NoResize | 
                ImGuiWindowFlags_AlwaysAutoResize |
                ImGuiWindowFlags_NoMove);
    

    if (ImGui::TreeNode(Scenes::currentSceneID.c_str())) {
        for (const auto& object :Scenes::currentScene->objects) {
            const char* objID = ("##" + object->name).c_str();

            if (ImGui::TreeNode(object->name.c_str())) {

                ImGui::SameLine(0.0f, 20.0f);
                ImGui::Checkbox(objID, &object->simulate);

                if (object->objectType == "star") { ImGui::BulletText("Star type: %c", object->light->starType); }
                ImGui::BulletText("Mass: %g t", (double)object->mass);
                ImGui::BulletText("Radius: %.0f km", (double)object->radius);
                ImGui::BulletText("Velocity: %.2f km/s", (double)glm::length(object->velocity));
                ImGui::TreePop();
            }
            else {
                ImGui::SameLine(0.0f, 20.0f);
                ImGui::Checkbox(objID, &object->simulate);
            }
        }
        ImGui::TreePop();
    }

    ImGui::PopFont();

    ImGui::End();
}

// checks if the mouse is above a ui element if so, disables camera controls for the frame
void GuiCameraInterruption() {
    if (ImGui::IsAnyItemActive() || ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow)) {
        supressCameraControls = true;
    }
} 