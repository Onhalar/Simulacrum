#include <config.hpp>
#include <globals.hpp>
#include <types.hpp>

#include <debug.hpp>
#include <FormatConsole.hpp>

#include <json.hpp>

#include <simObject.hpp>
#include <customMath.hpp>

#include <climits>

#include <simObject.hpp>

#include <scenes.hpp>

void advanceObjectPositions() {
    for (const auto& simObject : Scenes::currentScene) {
        advanceObjectPositon(simObject);
    }
}
