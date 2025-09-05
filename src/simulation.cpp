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

#include <physicsThread.hpp>

inline void advanceObjectPosition(simulationObject* simObject);
inline void advanceObjectPosition(simulationObject* simObject, glm::dvec3 realPosition);

void calcGravVelocity(simulationObject* currentObject, unsigned int groupID);

void simulateStep();

void physicsThreadFunction() {
    using namespace std::chrono;

    physicsDeltaTime = 1.0 / (double)physicsSteps;

    auto previousTime = steady_clock::now();
    double accumulator = 0.0;

    while (physicsRunning) {
        auto currentTime = steady_clock::now();
        duration<double> frameTime = currentTime - previousTime; // time elapsed since the previous loop iteration
        previousTime = currentTime;

        accumulator += frameTime.count(); // add elapsed time to the unprocessed-physics-time accumulator

        while (accumulator >= physicsDeltaTime) { // while there's at least one full physics step worth of time available
            {
                std::lock_guard<std::mutex> lock(physicsMutex);
                simulateStep(); // advance simulation by one fixed physics step (physicsDeltaTime)
            }
            accumulator -= physicsDeltaTime; // consume one physics step's worth of accumulated time
        }

        std::this_thread::sleep_for(milliseconds(1)); // tiny sleep to avoid busy-waiting the CPU
    }
}

bool firstFrame = true;

// Master Simulation Step Function
void simulateStep() {

    /*if (firstFrame) {
        for (auto& simObject : Scenes::currentScene->objects) { simObject->realVelocity *= simulationSpeed; }
        firstFrame = false;
    }*/

    if (deltaTime == 0.0) { return; }

    for (unsigned int groupID = 0; groupID < Scenes::currentScene->groups.size(); ++groupID) {
        const auto& currentGroup = Scenes::currentScene->groups[groupID];

        for (const auto& simObject : Scenes::currentScene->groups[groupID]) {

            for (int step = 0; step < phyiscsSubsteps; step++) {
                calcGravVelocity(simObject, groupID);
                advanceObjectPosition(simObject);
            }
        }
    }
}

// -----------------===[ Helper Functions ]===-----------------

inline void advanceObjectPosition(simulationObject* simObject) {
    double deltaSubStep = physicsDeltaTime  * simulationSpeed / (double)phyiscsSubsteps;

    //printVec3("acceleration", simObject->realAcceleration * physicsDeltaTime);

    simObject->realVelocity += simObject->realAcceleration * deltaSubStep;

    simObject->realPosition += simObject->realVelocity * deltaSubStep;
    simObject->position = simObject->realPosition / currentScale;

    simObject->realAcceleration = glm::dvec3(0.0);
}


inline void advanceObjectPosition(simulationObject* simObject, glm::dvec3 realPosition) {

    simObject->realPosition = realPosition;

    simObject->position = simObject->realPosition / currentScale;

    simObject->realAcceleration = glm::dvec3(0.0);
}

void calcGravVelocity(simulationObject* currentObject, unsigned int groupID) {
    glm::dvec3 fullGravPullAcceleration = glm::dvec3(0.0);

    for (const auto& simObject : Scenes::currentScene->groups[groupID]) {
        // Skip self-gravity
        if (currentObject == simObject) { continue; }
        
        // Get distance in simulation units and convert to meters
        units::meters distance = glm::distance(currentObject->realPosition, simObject->realPosition) * 1'000.0;
        units::kilograms comparisonObjectMass = units::manual_cast<units::kilograms>(simObject->mass, 1'000.0);
        
        double gravitationalAcceleration = GRAVITATIONAL_CONSTANT * (comparisonObjectMass) / (double)(distance * distance);
        
        glm::dvec3 direction = glm::normalize(simObject->realPosition - currentObject->realPosition);

        //std::cout << "Direction: " << direction.x << ", " << direction.y << ", " << direction.z << std::endl;
        
        glm::dvec3 gravPullAcceleration = direction * (gravitationalAcceleration / 1'000.0);
        fullGravPullAcceleration += gravPullAcceleration;
    }

    // Store acceleration in km/s²
    currentObject->realAcceleration = fullGravPullAcceleration;
}
