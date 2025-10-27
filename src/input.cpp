#include <globals.hpp>
#include <state.hpp>
#include <renderDefinitions.hpp>

#include <unordered_map>
#include <GLFW/glfw3.h>

std::unordered_map<int, bool> wasPressed;

void handleInputs() {
    bool isPressedEsc = glfwGetKey(mainWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS;
    if (isPressedEsc && !wasPressed[GLFW_KEY_ESCAPE]) {
        showMenu = !showMenu;
        if (!showMenu) {
            if (mainState == state::paused) { transitionState(state::running); }
        }
        else {
            if (mainState != state::paused) { transitionState(state::paused); }
        }
    }
    wasPressed[GLFW_KEY_ESCAPE] = isPressedEsc | currentCamera->focused;

    bool isPressedSpace = glfwGetKey(mainWindow, GLFW_KEY_SPACE) == GLFW_PRESS;
    if (isPressedSpace && !wasPressed[GLFW_KEY_SPACE]) {
        if (!showMenu) {
            if (mainState != state::paused) { transitionState(state::paused); }
            else { transitionState(state::running); }
        }
    }
    wasPressed[GLFW_KEY_SPACE] = isPressedSpace | currentCamera->focused;
}