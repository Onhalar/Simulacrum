#include <config.hpp>
#include <globals.hpp>
#include <types.hpp>

#include <debug.hpp>
#include <FormatConsole.hpp>

#include <json.hpp>

#include <simObjectList.hpp>
#include <simObject.hpp>

#include <math.h>

using Json = nlohmann::json;

void setupSimObjects() {
    //SimObjects["pyramid"] = new simulationObject("planet", "pyramid");
    //SimObjects["sphere"] = new simulationObject("planet", "sphere");

    units::kilometers minObjectRadius = DBL_MAX;

    for (const auto& simObject : SimObjects) {
        simObject.second->calculateAproximateRadius();
        simObject.second->normalizeVertices(normalizedModelRadius);
        minObjectRadius = std::min(simObject.second->radius, minObjectRadius);
    }

    if (minObjectRadius > 0 && SimObjects.size() > 1) { // will be -1 if not all objects are present
        for (const auto& simObject : SimObjects) {
            double scaleFactor = (simObject.second->radius / minObjectRadius);

            simObject.second->scaleVertices( scaleFactor * modelScalingStrength );
            simObject.second->updateVertexRadius( simObject.second->vertexModelRadius * scaleFactor * modelScalingStrength );
        }
    }

    for (const auto& simObject : SimObjects) {
        simObject.second->model->sendBufferedVertices();
    }
}

void loadSimObjects(std::filesystem::path path) {
    if (debugMode) {
        std::cout << formatProcess("Loading") << " settings '" << formatPath(getFileName(path.string())) << "' ... ";
    }

    if (!filesystem::exists(path) && debugMode) {
        std::cout << formatError("FAILED") << "\n";
        std::cerr << "unable to open '" << formatPath(path.string()) << "'\n" << endl;
        return;
    }

    std::ifstream file(path);
    Json data;
    file >> data;
    file.close();

    std::string mandatoryData[] = {"shader", "model"};

    for (const auto& entry : data.items()) {

        auto entryKey = entry.key();
        auto entryValue = entry.value();

        // check if all mandatory items are present.
        bool mandatoryDataPresent = true;
        for (const auto& item : mandatoryData) {
            if (!entryValue.contains(item)) {
                mandatoryDataPresent = false;
                break;
            }
        }

        if (!mandatoryDataPresent) {
            if (debugMode) { std::cerr << formatError("ERROR") << ": in object " << formatRole(entry.key()) << " not all mandatory data is present." << std::endl; }
            continue;
        }

        SimObjects[entryKey] = new simulationObject(entryValue["shader"], entryValue["model"]);

        if (entryValue.contains("color")) {
            auto color = entryValue["color"];
            SimObjects[entryKey]->model->objectColor = glm::vec3( color[0].get<float>(), color[1].get<float>(), color[2].get<float>() );
        }
        if (entryValue.contains("radius")) {
            SimObjects[entryKey]->radius = entryValue["radius"];
        }
        if (entryValue.contains("mass") &&
            entryValue["mass"].contains("value") &&
            entryValue["mass"].contains("notation")
        ) {
                SimObjects[entryKey]->mass = entryValue["mass"]["value"].get<double>() * std::pow(10, entryValue["mass"]["notation"].get<unsigned int>());
        }
    }

}