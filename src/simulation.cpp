#include <config.hpp>
#include <globals.hpp>
#include <types.hpp>

#include <debug.hpp>
#include <FormatConsole.hpp>

#include <json.hpp>

#include <simObject.hpp>

#include <math.h>
#include <climits>

#include <simObject.hpp>

#include <scenes.hpp>

using Json = nlohmann::json;


// y       = 8   = 1000
// y - 1   = 7   = 0111

template <typename T>
bool isPowerOfTwo (const T& number) {
    static_assert(std::is_unsigned<T>::value, "Type must be unsigned"); // compile time check (static assert); value -> 0/1

    return number != 0 && (number & (number - 1)) == 0;
}

template <typename T>
T roundUpToPowerOfTwo(T& number) {
    static_assert(std::is_unsigned<T>::value, "Type must be unsigned");

    if ( number == 0 ) { return 1; }
    if ( number == std::numeric_limits<T>::max() ) { return ~(number >> 1); } // (>>) 1111 => (~) 0111 => 1000

    number--; // necesery because of if number is 1

    T temp = 1;

    for (; temp < number;) {
        temp <<= 1; // shift bit by one space to the left (equivalent *= 2) => 0001 => 0010
    }

    return temp;
}

units::kilometers exponentialScale(const units::kilometers& minValue, const units::kilometers& MaxValue, const units::kilometers& currentValue, const float& maxScale = maxScale) {
    if (minValue >= MaxValue /*Zero Division handler*/ || currentValue <= minValue) {
        return 1.0;
    }

    if (currentValue >= MaxValue) {
        return maxScale;
    }

    // Normalize the current value to a 0-1 range.
    double normalized_val = (currentValue - minValue) / (MaxValue - minValue);

    return std::pow(maxScale, normalized_val);
}











// ------------------------==[ SETUP ]==------------------------

void setupSceneObjects(const SceneID& sceneID, const bool& setAsActive = true) {
    //SimObjects["pyramid"] = new simulationObject("planet", "pyramid");
    //SimObjects["sphere"] = new simulationObject("planet", "sphere");

    lightQue.clear();

    units::kilometers minObjectRadius = DBL_MAX;
    units::kilometers MaxObjctRadius = DBL_MIN;

    for (const auto& simObject : Scenes::allScenes[sceneID]) {
        simObject->calculateAproximateRadius();
        simObject->normalizeVertices(normalizedModelRadius);

        minObjectRadius = std::min(simObject->radius, minObjectRadius);
        MaxObjctRadius = std::max(simObject->radius, MaxObjctRadius);
    }

    if (minObjectRadius > 0) { // will be -1 if not all objects are present
        for (const auto& simObject : Scenes::allScenes[sceneID]) {

            double scaleFactor;

            if (scalingMode == scalingType::realistic) {
                scaleFactor = (simObject->radius / minObjectRadius);
            }
            else if (scalingMode == scalingType::simlyfied) {
                scaleFactor = exponentialScale(minObjectRadius, MaxObjctRadius, simObject->radius);
            }

            simObject->scaleVertices(scaleFactor);
            simObject->vertexModelRadius = simObject->vertexModelRadius * scaleFactor;

            if (simObject->rotationSpeed != -1 && simulateObjectRotation) {
                double objectCircumference = 2.0 * PI * simObject->radius;

                simObject->vertexRotation = 360.0 * ( ( simObject->rotationSpeed / objectCircumference ) / 3600 ); // degrees per second
            }
        }
    }

    for (const auto& simObject : Scenes::allScenes[sceneID]) {
        if (simObject->light != nullptr) {
            lightQue[simObject->name] = simObject->light;
        }
    }

    for (const auto& simObject : Scenes::allScenes[sceneID]) {
        simObject->model->sendBufferedVertices();
    }

    // light positions will be updated in the main loop
}

void switchSceneAndCalculateObjects(const SceneID& sceneID) {
    setupSceneObjects(sceneID);
    Scenes::switchScene(sceneID);
}

void loadSimObjects(std::filesystem::path path) {
    if (debugMode) {
        std::cout << '\n' << formatProcess("Loading") << " objects '" << formatPath(getFileName(path.string())) << "' ... ";
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

        if (entryValue.contains("color") && entryValue["color"].size() == 3) {
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

        if (entryValue.contains("type")) {
            SimObjects[entryKey]->objectType = entryValue["type"].get<std::string>();
        }

        if (entryValue.contains("rotation") && SimObjects[entryKey]->radius != -1) {
            SimObjects[entryKey]->rotationSpeed = entryValue["rotation"].get<double>();
        }
        else {
            SimObjects[entryKey]->vertexRotation = 0.003992; //360 * ((earthRotationKmH / (EarthRadius*PI*2)) / 3600) -> approximate Earth's rotation degrees / second
        }

        if (entryValue.contains("light")) {
            glm::vec3 color; float intensity;
            if (entryValue["light"].contains("intensity")) {
                intensity = entryValue["light"]["intesity"].get<float>();
            }
            else {
                intensity = 1.5f;
            }
            
            if (entryValue["light"].contains("color")){
                auto lightColor = entryValue["light"]["color"];
                color = glm::vec3( lightColor[0].get<float>(), lightColor[1].get<float>(), lightColor[2].get<float>() );
            } else {
                color = SimObjects[entryKey]->model->objectColor;
            }

            SimObjects[entryKey]->light = new LightObject(SimObjects[entryKey]->position, color, intensity);            
        }
        else if (SimObjects[entryKey]->objectType == "star") {
            SimObjects[entryKey]->light = new LightObject();
        }
    }

    if (debugMode) { std::cout << formatSuccess("Done") << std::endl; }

}

void loadPhysicsScene(std::filesystem::path path) {
    if (debugMode) {
        std::cout << '\n' << formatProcess("Loading") << " scenes '" << formatPath(getFileName(path.string())) << "' ... ";
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
    
    std::stringstream debugBuffer;
    
    for (const auto& entry : data.items()) {

        auto sceneID = entry.key();
        auto entryValue = entry.value();

        scene currentScene;

        for (const auto& simObject : entryValue) {
            
            if (!simObject.contains("object")) {
                if (debugMode) { debugBuffer << formatError("ERROR") << ": In scene '" << formatRole(sceneID) << "' and object instance not specified ... " << formatWarning("Skipping") << '\n'; }
                continue;
            }

            SimObjectID objectID = simObject["object"].get<std::string>();
            if (SimObjects.find(objectID) == SimObjects.end()) {
                if (debugMode) { debugBuffer << formatError("ERROR") << ": Object '" << formatRole(objectID) << "' in scene '" << formatRole(sceneID) << "' ... " << formatWarning("Skipping") << '\n'; }
                continue;
            }
            
            simulationObject* currentSimObject = SimObjects[objectID];

            if (simObject.contains("position")) {
                if (simObject["position"].size() == 3) {
                    auto position = simObject["position"];
                    currentSimObject->position = glm::vec3(position[0].get<float>(), position[1].get<float>(), position[2].get<float>());
                }
                else {
                    if (debugMode) { debugBuffer << formatWarning("WARNING") << ": In scene '" << formatRole(sceneID) << "' position has incorrect format " << formatProcess("[x, y, z]") << " for object '" << formatRole(objectID) << "' ... " << formatProcess("Loading defaults") << '\n'; }
                    currentSimObject->position = glm::dvec3(0);
                }
            }
            else {
                if (debugMode) { debugBuffer << formatWarning("WARNING") << ": In scene '" << formatRole(sceneID) << "' position not found for object '" << formatRole(objectID) << "' ... " << formatProcess("Loading defaults") << '\n'; }
                currentSimObject->position = glm::dvec3(0);
            }

            if (simObject.contains("velocity")) {
                if (simObject["velocity"].size() == 3) {
                    auto position = simObject["velocity"];
                    currentSimObject->velocity = glm::dvec3(position[0].get<double>(), position[1].get<double>(), position[2].get<double>());
                }
                else {
                    if (debugMode) { debugBuffer << formatWarning("WARNING") << ": In scene '" << formatRole(sceneID) << "' velocity has incorrect format " << formatProcess("[x, y, z]") << " for object '" << formatRole(objectID) << "' ... " << formatProcess("Loading defaults") << '\n'; }
                    currentSimObject->velocity = glm::dvec3(0);
                }
            }
            else {
                if (debugMode) { debugBuffer << formatWarning("WARNING") << ": In scene '" << formatRole(sceneID) << "' velocity not found for object '" << formatRole(objectID) << "' ... " << formatProcess("Loading defaults") << '\n'; }
                currentSimObject->velocity = glm::dvec3(0);
            }

            currentScene.insert(currentSimObject);
        }

        if (currentScene.empty()) {
            debugBuffer << formatError("ERROR") << ": In scene '" << formatRole(sceneID) << "' no objects were found or passed tests ... " << formatError("Skipping") << '\n';
        }
        else {
            Scenes::allScenes[sceneID] = currentScene;
            currentScene.clear(); // likely not necesery but just to be sure
        }
        
    }

    std::string debugBufferContents = debugBuffer.str();
    if (debugBufferContents.empty()) {
        std::cout << formatSuccess("Done") << std::endl;
    }
    else {
        std::cout << formatWarning("Done with exceptions") << '\n' << debugBufferContents << std::endl;
    }

    // setting up default fallback scene
    if (Scenes::allScenes.empty()) {
        if (debugMode) { std::cout << formatWarning("WARNING") << ": No scenes were loaded / found" << std::endl; }
    }
    else {
        switchSceneAndCalculateObjects( (*Scenes::allScenes.begin()).first );
    }

}