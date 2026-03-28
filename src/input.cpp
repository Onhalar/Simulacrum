#include "camera.hpp"
#include "scenes.hpp"
#include <config.hpp>
#include <globals.hpp>
#include <state.hpp>
#include <renderDefinitions.hpp>

#include <unordered_map>
#include <GLFW/glfw3.h>

std::unordered_map<unsigned int, bool> wasPressed;

inline bool isJustPressed(const unsigned int& GlfwKey, const unsigned int& WasPressedFlags );
inline bool isJustPressed(const unsigned int& GlfwKey ); 

void handleInputs() {

    if (isJustPressed(GLFW_KEY_ESCAPE)) {
        // Unfocusing is handled by camera
        if (currentCamera->focused) { return; }

        if (showBackgroundChanger) {
            showBackgroundChanger = false;
            return;
        }

        if (showScenePicker && Scenes::currentScene) {
            showScenePicker = false;
            return;
        }

        showMenu = !showMenu;

        if (showMenu) {
            // Opening menu: remember if we were paused, then pause
            wasStatePausedBeforeMenu = (mainState == state::paused);
            if (mainState != state::paused) { transitionState(state::paused); }
        } else {
            // Closing menu: restore previous state
            if (!wasStatePausedBeforeMenu) { transitionState(state::running); }
            wasStatePausedBeforeMenu = false;
        }
    }

    if (isJustPressed(GLFW_KEY_SPACE, currentCamera->focused)) {
        if (!showMenu && !showBackgroundChanger) {
            if (mainState != state::paused) { transitionState(state::paused); }
            else { transitionState(state::running); }
        }
    }

    if (isJustPressed(GLFW_KEY_F11)) {
        if (fullscreen) { exitFullscreen(); }
        else { enterFullscreen(); }
        settingsUpdated = true;
    }
}



inline bool isJustPressed(const unsigned int& GlfwKey ) {
    bool output = false;

    bool isPressed = glfwGetKey(mainWindow, GlfwKey) == GLFW_PRESS;
    if (isPressed && !wasPressed[GlfwKey]) { output = true; }
    wasPressed[GlfwKey] = isPressed;

    return output;
}

// was pressed flags just act as "wasPressed = isPressed or [flags]"
inline bool isJustPressed(const unsigned int& GlfwKey, const unsigned int& WasPressedFlags ) {
    bool output = false;

    bool isPressed = glfwGetKey(mainWindow, GlfwKey) == GLFW_PRESS;
    if (isPressed && !wasPressed[GlfwKey]) { output = true; }
    wasPressed[GlfwKey] = isPressed | WasPressedFlags;

    return output;
}