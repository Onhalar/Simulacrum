#include <cmath>
#include <debug.hpp>
#include <globals.hpp>
#include <renderDefinitions.hpp>

// ImGui includes
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>

// use to display the current scene
#include <scenes.hpp>
#include <sstream>

void updateDefaultFontSize(const float& fontSize) {
    static ImFontConfig config;
    static ImGuiIO& io = ImGui::GetIO();

    config.SizePixels = fontSize;
    io.Fonts->AddFontDefault(&config);
}

void renderGui() {
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Get the viewport size to position in top right
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 10, 10), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
    
    ImGui::SetNextWindowBgAlpha(0.35f); // Semi-transparent so you can see through it
    ImGui::Begin("Overlay", nullptr, 
                ImGuiWindowFlags_NoTitleBar | 
                ImGuiWindowFlags_NoResize | 
                ImGuiWindowFlags_AlwaysAutoResize |
                ImGuiWindowFlags_NoMove);
    

    if (ImGui::TreeNode(Scenes::currentSceneID.c_str())) {
        for (const auto& object :Scenes::currentScene->objects) {
            if (ImGui::TreeNode(object->name.c_str())) {
                std::stringstream massBuffer;
                std::stringstream radiusBuffer;
                std::stringstream velocityBuffer;

                massBuffer << "Mass: " << object->mass << " t";
                radiusBuffer << "Radius: " << object->radius << " km";
                velocityBuffer << "Velocity: " << std::sqrt( (object->realVelocity.x * object->realVelocity.x) + (object->realVelocity.y * object->realVelocity.y) + (object->realVelocity.z * object->realVelocity.z) ) << " km/s";

                ImGui::BulletText(massBuffer.str().c_str());
                ImGui::BulletText(radiusBuffer.str().c_str());
                ImGui::BulletText(velocityBuffer.str().c_str());
                ImGui::TreePop();
            }
        }
        ImGui::TreePop();
    }

    ImGui::End();

    // Rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

// checks if the mouse is above a ui element if so, disables camera controls for the frame
void GuiCameraInterruption() {
    if (ImGui::IsAnyItemActive() || ImGui::IsAnyItemHovered() || ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow)) {
        supressCameraControls = true;
    }
}