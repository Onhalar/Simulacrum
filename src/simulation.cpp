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

void calcGravVelocity(simulationObject* currentObject);
void calculateVelocity(simulationObject* simObject);

void simulateStep();

void physicsThreadFunction() {
    using namespace std::chrono;

    physicsDeltaTime = 1.0 / (double)physicsSteps;

    auto previousTime = steady_clock::now();
    double accumulator = 0.0;

    while (physicsRunning) {
        auto currentTime = steady_clock::now();
        duration<double> frameTime = currentTime - previousTime;
        previousTime = currentTime;

        accumulator += frameTime.count();

        while (accumulator >= physicsDeltaTime) {
            {
                std::lock_guard<std::mutex> lock(physicsMutex);
                simulateStep(); // update physics
            }
            accumulator -= physicsDeltaTime;
        }

        std::this_thread::sleep_for(milliseconds(1)); // avoid 100% CPU
    }
}


bool firstFrame = true;

// Master Simulation Step Function
void simulateStep() {

    if (deltaTime == 0.0) { return; }

    phyiscsBufferedFrames = std::max(1u, phyiscsBufferedFrames);

    for (const auto& simObject : Scenes::currentScene) {

        if (firstFrame) { simObject->realVelocity *= physicsDeltaTime * simulationSpeed; }

        glm::dvec3 startPosition = simObject->realPosition;

        for (int bufferedFrame = 0; bufferedFrame < phyiscsBufferedFrames; bufferedFrame++) {
            for (int step = 0; step < phyiscsSubsteps; step++) {
                calcGravVelocity(simObject);
                advanceObjectPosition(simObject);
            }
        }

        if (phyiscsBufferedFrames > 1) {
            // compute the average position over buffered frames
            glm::dvec3 avgPosition = startPosition + (simObject->realPosition - startPosition) / (double)phyiscsBufferedFrames;
            advanceObjectPosition(simObject, avgPosition);

            //std::cout << "velocity: " << simObject->realVelocity.x << ", " <<simObject->realVelocity.y << ", " <<simObject->realVelocity.z << std::endl;
            //std::cout << "deltaTime: " << deltaTime << " simulationSpeed: " << simulationSpeed << std::endl;
        }
    }

    firstFrame = false;
}

// -----------------===[ Helper Functions ]===-----------------

inline void advanceObjectPosition(simulationObject* simObject) {
    auto deltaSubStep = ((physicsDeltaTime * simulationSpeed) / (double)phyiscsSubsteps);

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

void calcGravVelocity(simulationObject* currentObject) {
    glm::dvec3 fullGravPullAcceleration = glm::dvec3(0.0);

    for (const auto& simObject : Scenes::currentScene) {
        // Skip self-gravity
        if (currentObject == simObject) { continue; }
        
        // Get distance in simulation units and convert to meters
        units::meters distance = glm::distance(currentObject->realPosition, simObject->realPosition) * 1'000.0;
        units::kilograms comparisonObjectMass = simObject->mass * 1'000.0;
        
        double gravitationalAcceleration = GRAVITATIONAL_CONSTANT * (comparisonObjectMass) / (distance * distance);
        
        glm::dvec3 direction = glm::normalize(simObject->realPosition - currentObject->realPosition);

        //std::cout << "Direction: " << direction.x << ", " << direction.y << ", " << direction.z << std::endl;
        
        glm::dvec3 gravPullAcceleration = direction * (gravitationalAcceleration / 1'000.0);
        fullGravPullAcceleration += gravPullAcceleration;
    }

    // Store acceleration in km/s²
    currentObject->realAcceleration = fullGravPullAcceleration;
}
