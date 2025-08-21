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

void advanceObjectPositon(simulationObject* simObject);
void calcGravVelocity(simulationObject* currentObject);


// Master Simulation Step Function
void simulateStep() {

    for (int step = 0; step < phyiscsSubsteps; step++) {
        for (const auto& simObject : Scenes::currentScene) {

            calcGravVelocity(simObject);

            advanceObjectPositon(simObject);

        }
    }

}


// -----------------===[ Helper Functions ]===-----------------


void advanceObjectPositon(simulationObject* simObject) {

    auto deltaSubStep = (deltaTime / phyiscsSubsteps);

    std::cout << "Velocity: " << simObject->realVelocity.x << ", " << simObject->realVelocity.y << ", " << simObject->realVelocity.z << std::endl;
    //std::cout << "Position: " << simObject->realPosition.x << ", " << simObject->realPosition.y << ", " << simObject->realPosition.z << std::endl;

    simObject->realVelocity += simObject->realAcceleration  * deltaSubStep;
    simObject->realPosition += simObject->realVelocity * deltaSubStep * simulationSpeed;

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
