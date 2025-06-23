#ifndef UPDATE_RUNNING_CONFIG_HEADER
#define UPDATE_RUNNING_CONFIG_HEADER

#include <config.hpp>
#include <globals.hpp>

class updateRunningConfig {
    public:
        static void updateMaxFrameRate(int newMaxFrameRate) {
            frameDuration = nanoseconds(1'000'000'000 / newMaxFrameRate);
        }

        static void updateVSync(int newVSyncValue) {
            glfwSwapInterval(newVSyncValue);
        }

        static void updateAllFromSet() {
            updateVSync(VSync);
            updateMaxFrameRate(maxFrameRate);
        }
};

#endif // UPDATE_RUNNING_CONFIG_HEADER