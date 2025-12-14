#ifndef MAIN_STATE_HEADER
#define MAIN_STATE_HEADER

#include <globals.hpp>
#include <physicsThread.hpp>

#include <unistd.h>
#include <GLFW/glfw3.h>

enum state : unsigned char {
    running,
    paused,
    starting,
    loading,
    stopping,
};

inline state mainState = state::starting;

inline void transitionState(state newState) {
    if (newState == mainState) { return; }

    if (mainState == state::running) {
        if (newState == state::paused) {
            pausePhysicsThread = true;
            mainState = state::paused;
        }
        else if (newState == state::loading) {
            pausePhysicsThread = true;
            mainState = state::loading;
        }
    }
    else if (mainState == state::paused) {
        if (newState == state::running) {
            pausePhysicsThread = false;
            mainState = state::running;
        }
        else if (newState == state::loading) {
            pausePhysicsThread = true;
            mainState = state::loading;
        }
    }
    else if (mainState == state::loading) {
        if (newState == state::running || newState == state::paused) {
            pausePhysicsThread = false;
        }
    }
    else if (mainState == state::starting) {
        if (newState == state::paused) {
            pausePhysicsThread = true;
            mainState = state::paused;
        }
    }

    // Any current state
    if (newState == state::stopping) {
        glfwSetWindowShouldClose(mainWindow, true);
        mainState = state::stopping;
    }
}

#endif // MAIN_STATE_HEADER