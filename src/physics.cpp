#include <config.hpp>
#include <globals.hpp>
#include <types.hpp>

#include <simObjectList.hpp>

void setupPhysics() {
    SimObjects["pyramid"] = new simulationObject("planet", "pyramid");
    SimObjects["sphere"] = new simulationObject("planet", "sphere");

    units::kilometers minObjectRadius = DBL_MAX;

    for (const auto& simObject : SimObjects) {
        simObject.second->calculateAproximateRadius();
        simObject.second->normalizeVertices(normalizedModelRadius);
        minObjectRadius = std::min(simObject.second->radius, minObjectRadius);
    }

    /*for (const auto& simObject : SimObjects) {
        double scaleFactor = (simObject.second->radius / minObjectRadius);

        simObject.second->scaleVertices( scaleFactor * modelScalingStrength );
        simObject.second->updateVertexRadius( simObject.second->vertexModelRadius * scaleFactor * modelScalingStrength );
    }*/

    for (const auto& simObject : SimObjects) {
        simObject.second->model->sendBufferedVertices();
    }
}