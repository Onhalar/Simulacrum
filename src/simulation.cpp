#include "state.hpp"
#include "units.hpp"
#include <chrono>
#include <config.hpp>
#include <globals.hpp>
#include <string>
#include <thread>
#include <types.hpp>

#include <debug.hpp>
#include <FormatConsole.hpp>

#include <json.hpp>

#include <simObject.hpp>
#include <customMath.hpp>

#include <scenes.hpp>

#include <physicsThread.hpp>
#include <unistd.h>
#include <vector>

void advanceObjectPosition(SnapObj* simObject, glm::dvec3 newAcceleration);
glm::dvec3 calcGravVelocity(const SnapObj* currentObject, const std::vector<SnapObj*>& group);
void simulateStep(Snapshot* snapshot);



void physicsThreadFunction() {
    static Snapshot* snapshot = new Snapshot();

    using namespace std::chrono;
    bool wasPaused = false;

    physicsDeltaTime = 1.0 / (double)physicsSteps;

    auto previousTime = steady_clock::now();
    double accumulator = 0.0;

    while (physicsRunning) {

        if (pausePhysicsThread) { std::this_thread::sleep_for(chrono::microseconds(100)); wasPaused = true; continue; }
        else if (wasPaused) { previousTime = steady_clock::now(); wasPaused = false; }

        auto currentTime = steady_clock::now();
        duration<double> frameTime = currentTime - previousTime; // time elapsed since the previous loop iteration
        previousTime = currentTime;

        accumulator += frameTime.count(); // add elapsed time to the unprocessed-physics-time accumulator

        while (accumulator >= physicsDeltaTime) { // while there's at least one full physics step worth of time available
            snapshot->takeSnapshot();

            simulateStep(snapshot); // advance simulation by one fixed physics step (physicsDeltaTime)
            accumulator -= physicsDeltaTime; // consume one physics step's worth of accumulated time

            snapshot->updateOrigin();
        }

        std::this_thread::sleep_for(milliseconds(1)); // tiny sleep to avoid busy-waiting the CPU
    }

    delete snapshot;
}

// Master Simulation Step Function
void simulateStep(Snapshot* snapshot) {

    if (deltaTime == 0.0) { return; }

    for (int step = 0; step < phyiscsSubsteps; step++) {
        for (const auto& currentGroup : snapshot->groups) {
            for (const auto simObject : currentGroup) {
                if (!simObject->simulate) { continue; }

                glm::dvec3 newAcceleration = calcGravVelocity(simObject, currentGroup);
                advanceObjectPosition(simObject, newAcceleration);
            }
        }
    }
}

// -----------------===[ Helper Functions ]===-----------------

void advanceObjectPosition(SnapObj* simObject, glm::dvec3 newAcceleration) {
    double deltaSubStep = physicsDeltaTime * simulationSpeed / (double)phyiscsSubsteps;

    if (simObject->firstPass) {
        // Euler step to initialize
        simObject->acceleration = newAcceleration;
        simObject->velocity += newAcceleration * deltaSubStep;
        simObject->position += simObject->velocity * deltaSubStep;
        simObject->firstPass = false;
    } 
    else {
        // Varlet for continuous

        // velocity Verlet integration
        simObject->position += simObject->velocity * deltaSubStep + 0.5 * simObject->acceleration * deltaSubStep * deltaSubStep;
        
        // update velocity
        simObject->velocity += 0.5 * (simObject->acceleration + newAcceleration) * deltaSubStep;
        
        // store new acceleration for next iteration
        simObject->acceleration = newAcceleration;
    }
    
    simObject->vertPosition = simObject->position / (simulationMode == simulationType::simplified ? simObject->distanceScale : currentScale);
}

glm::dvec3 calcGravVelocity(const SnapObj* currentObject, const std::vector<SnapObj*>& group) {
    glm::dvec3 fullGravPullAcceleration = glm::dvec3(0.0);

    for (const auto& simObject : group) {
        if (!simObject->simulate) { continue; } // remove non-simulated object's influence

        if (currentObject == simObject) { continue; } // Skip self-gravity
        if (currentObject->position == simObject->position) { continue; } // skip distane calculation errors (inf)
        
        // Get distance in simulation units and convert to meters
        units::meters distance = glm::distance(currentObject->position, simObject->position) * 1'000.0;
        units::kilograms comparisonObjectMass = units::manual_cast<units::kilograms>(simObject->mass, 1'000.0);
        
        double gravitationalAcceleration = GRAVITATIONAL_CONSTANT * (comparisonObjectMass) / (double)(distance * distance);

        glm::dvec3 direction = glm::normalize(simObject->position - currentObject->position);
        
        glm::dvec3 gravPullAcceleration = direction * (gravitationalAcceleration / 1'000.0);
        fullGravPullAcceleration += gravPullAcceleration;
    }

    return fullGravPullAcceleration;
}
