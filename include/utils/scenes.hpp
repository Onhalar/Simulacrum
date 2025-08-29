#ifndef PHYSICS_SCENE_CLASS_HEADER
#define PHYSICS_SCENE_CLASS_HEADER

#include <simObject.hpp>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <types.hpp>
#include <customMath.hpp>

#include <renderDefinitions.hpp>
#include <math.h>


using sceneGroup = std::unordered_set<simulationObject*>;

struct scene {
    std::unordered_set<simulationObject*> objects;
    std::vector<sceneGroup> groups;
};

using sceneList = std::unordered_map<std::string, scene*>;

class Scenes {
    public:
        inline static sceneList allScenes; // inline propperly initializes it for some reason
        inline static scene* currentScene;

    static void switchScene(SceneID sceneID) {
        currentScene = allScenes[sceneID];
    }
};

void setupSceneObjects(const SceneID& sceneID, const bool& setAsActive = true) {
    //SimObjects["pyramid"] = new simulationObject("planet", "pyramid");
    //SimObjects["sphere"] = new simulationObject("planet", "sphere");

    lightQue.clear();

    currentScale = 0.0;

    renderScaleDistortion = max(renderScaleDistortion, 1.0);

    units::kilometers minObjectRadius = DBL_MAX;
    units::kilometers MaxObjctRadius = DBL_MIN;

    for (const auto& simObject : Scenes::allScenes[sceneID]->objects) {
        simObject->calculateAproximateRadius();
        simObject->normalizeVertices(normalizedModelRadius);

        minObjectRadius = std::min(simObject->radius, minObjectRadius);
        MaxObjctRadius = std::max(simObject->radius, MaxObjctRadius);
    }

    if (minObjectRadius > 0) { // will be -1 if not all objects are present
        for (const auto& simObject : Scenes::allScenes[sceneID]->objects) {

            double scaleFactor;

            if (simulationMode == simulationType::realistic) {
                scaleFactor = (simObject->radius / minObjectRadius);
                if (!currentScale) { currentScale = (minObjectRadius / (double)normalizedModelRadius) * renderScaleDistortion; }
            }
            else if (simulationMode == simulationType::simplified) {
                scaleFactor = exponentialScale(minObjectRadius, MaxObjctRadius, simObject->radius);
                if (!currentScale) { currentScale = ( (minObjectRadius /* /1 */ + (MaxObjctRadius / (double)maxScale)) /2 ) * renderScaleDistortion; }
            }

            simObject->scaleVertices(scaleFactor);
            simObject->vertexModelRadius *= scaleFactor;

            if (simObject->rotationSpeed != -1 && simulateObjectRotation) {
                double objectCircumference = 2.0 * PI * simObject->radius;

                simObject->vertexRotation = 360.0 * ( ( simObject->rotationSpeed / objectCircumference ) / 3600 ); // degrees per second
            }
        }
    }
    else {
        currentScale = 1.0;
    }

    for (const auto& simObject : Scenes::allScenes[sceneID]->objects) {
        if (simObject->light != nullptr) {
            lightQue[simObject->name] = simObject->light;
        }
    }

    for (const auto& simObject : Scenes::allScenes[sceneID]->objects) {
        simObject->model->sendBufferedVertices();
    }

    // light positions will be updated in the main loop
}

void adjustCameraToScene(const SceneID& sceneID) {
    double maxDistance = 0.0;

    for (const auto& object : Scenes::allScenes[sceneID]->objects) {
        if (object->realPosition != glm::dvec3(0.0)) {
            double distance = glm::distance(glm::dvec3(0.0), object->realPosition);
            maxDistance = (distance > maxDistance? distance : maxDistance);
        }
    }

    maxDistance /= currentScale;

    currentCamera->position.z = (maxDistance / std::tan(glm::radians(currentCamera->FOVdeg) / 2.0));
}

void switchSceneAndCalculateObjects(const SceneID& sceneID) {
    setupSceneObjects(sceneID);
    Scenes::switchScene(sceneID);
    adjustCameraToScene(sceneID);
}

#endif // PHYSICS_SCENE_CLASS_HEADER