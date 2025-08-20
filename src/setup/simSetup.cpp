#include <config.hpp>
#include <types.hpp>
#include <renderDefinitions.hpp>

#include <simObject.hpp>
#include <scenes.hpp>

#include <debug.hpp>
#include <FormatConsole.hpp>
#include <paths.hpp>

#include <math.h>

using Json = nlohmann::json;

void loadSimObjects(std::filesystem::path path);
void loadPhysicsScene(std::filesystem::path path);

void setupSimulation() {
    loadSimObjects(projectPath(simObjectsConfigPath));

    loadPhysicsScene(projectPath(physicsScenesPath));
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
            SimObjects[entryKey]->radius = entryValue["radius"].get<units::kilometers>();
        }

        if (entryValue.contains("mass")) {
            SimObjects[entryKey]->mass = entryValue["mass"].get<units::tons>();
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
                    currentSimObject->realPosition = glm::vec3(position[0].get<double>(), position[1].get<double>(), position[2].get<double>());
                }
                else {
                    if (debugMode) { debugBuffer << formatWarning("WARNING") << ": In scene '" << formatRole(sceneID) << "' position has incorrect format " << formatProcess("[x, y, z]") << " for object '" << formatRole(objectID) << "' ... " << formatProcess("Loading defaults") << '\n'; }
                    currentSimObject->realPosition = glm::dvec3(0);
                }
            }
            else {
                if (debugMode) { debugBuffer << formatWarning("WARNING") << ": In scene '" << formatRole(sceneID) << "' position not found for object '" << formatRole(objectID) << "' ... " << formatProcess("Loading defaults") << '\n'; }
                currentSimObject->position = glm::dvec3(0);
            }

            if (simObject.contains("velocity")) {
                if (simObject["velocity"].size() == 3) {
                    auto position = simObject["velocity"];
                    currentSimObject->realVelocity = glm::dvec3(position[0].get<double>(), position[1].get<double>(), position[2].get<double>());
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