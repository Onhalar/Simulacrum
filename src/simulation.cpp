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

inline void advanceObjectPositon(simulationObject* simObject);
inline void advanceObjectPositon(simulationObject* simObject, glm::dvec3 realPosition);

void calcGravVelocity(simulationObject* currentObject);
void calculateVelocity(simulationObject* simObject);

// Master Simulation Step Function
void simulateStep() {

    phyiscsBufferedFrames = std::max(1u, phyiscsBufferedFrames);

    for (const auto& simObject : Scenes::currentScene) {

        glm::dvec3 startPosition = simObject->realPosition;

        for (int bufferedFrame = 0; bufferedFrame < phyiscsBufferedFrames; bufferedFrame++) {
            for (int step = 0; step < phyiscsSubsteps; step++) {
                calcGravVelocity(simObject);
                advanceObjectPositon(simObject);
            }
        }

        if (phyiscsBufferedFrames >= 2) {
            // compute the average position over buffered frames
            glm::dvec3 avgPosition = startPosition + (simObject->realPosition - startPosition) / (double)phyiscsBufferedFrames;
            advanceObjectPositon(simObject, avgPosition);
        }
    }
}

// -----------------===[ Helper Functions ]===-----------------

inline void advanceObjectPositon(simulationObject* simObject) {
    auto deltaSubStep = (deltaTime / phyiscsSubsteps);

    simObject->realVelocity += simObject->realAcceleration  * deltaSubStep;

    simObject->realPosition += simObject->realVelocity * deltaSubStep * simulationSpeed;

    simObject->position = simObject->realPosition / currentScale;

    simObject->realAcceleration = glm::dvec3(0.0);
}

inline void advanceObjectPositon(simulationObject* simObject, glm::dvec3 realPosition) {

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
