#include <config.hpp>
#include <types.hpp>
#include <renderDefinitions.hpp>

#include <simObject.hpp>
#include <scenes.hpp>

#include <debug.hpp>
#include <FormatConsole.hpp>
#include <paths.hpp>

#include <math.h>
#include <optional>
#include <variant>

#include <color.hpp>

using Json = nlohmann::json;
using errorCode = std::string;

void loadSimObjects(std::filesystem::path path);
void loadPhysicsScene(std::filesystem::path path);

void setupSimulation() {
    loadSimObjects(projectPath(simObjectsConfigPath));

    loadPhysicsScene(projectPath(physicsScenesPath));
}

std::optional<ShaderID> generateFallbackShaderID () {
    std::optional<ShaderID> output;
    if (!Shaders.empty()) {
        output = (*Shaders.begin()).first;
    }
    return output;
}
std::optional<ModelID> generateFallbackModelID () {
    std::optional<ModelID> output;
    if (!Models.empty()) {
        output = (*Models.begin()).first;
    }
    return output;
}

std::optional<ShaderID> resolveFallbackShaderID(std::string ObjectName, std::stringstream& debugBuffer, bool isMissing = false) {
    std::optional<ShaderID> shader;

    debugBuffer << formatError("ERROR") << ": in object '" << formatPath(ObjectName) << "' shader value is " << (isMissing ? "Missing" : "incorrect") << " ... " << formatProcess("Attempting to resolve") << " ... ";
    auto maybeShader = generateFallbackShaderID();
    if (maybeShader.has_value()) {
        shader = maybeShader.value();
        debugBuffer << formatProcess("Loaded ") << "'" << formatPath(shader.value()) << "' " << formatRole("shader") << std::endl;
    }
    else {
        debugBuffer << formatError("FAILED") <<  " ... Skipping" << std::endl;
    }

    return shader;
}

std::optional<ShaderID> resolveFallbackModelID(std::string ObjectName, std::stringstream& debugBuffer, bool isMissing = false) {
    std::optional<ShaderID> model;

    debugBuffer << formatError("ERROR") << ": in object '" << formatPath(ObjectName) << "' model value is " << (isMissing ? "Missing" : "incorrect") << " ... " << formatProcess("Attempting to resolve") << " ... ";
    auto maybeModel = generateFallbackModelID();
    if (maybeModel.has_value()) {
        model = maybeModel.value();
        debugBuffer << formatProcess("Loaded ") << "'" << formatPath(model.value()) << "' " << formatRole("model") << std::endl;
    }
    else {
        debugBuffer << formatError("FAILED") <<  " ... Skipping" << std::endl;
    }

    return model;
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

    std::stringstream debugBuffer;

    for (const auto& entry : data.items()) {

        auto entryKey = entry.key();
        auto entryValue = entry.value();

        ShaderID shader;
        ModelID model;

        if (entryValue.contains("shader")) {
            if (Shaders.find(entryValue["shader"]) != Shaders.end()) {
                shader = entryValue["shader"];
            }
            else {
                auto maybeShader = resolveFallbackShaderID(entryKey, debugBuffer);
                if (maybeShader.has_value()) {
                    shader = maybeShader.value();
                }
                else { continue; }
            }
        }
        else {
            auto maybeShader = resolveFallbackShaderID(entryKey, debugBuffer, true);
            if (maybeShader.has_value()) {
                shader = maybeShader.value();
            }
            else { continue; }
        }

        if (entryValue.contains("model")) {
            if (Models.find(entryValue["model"]) != Models.end()) {
                model = entryValue["model"];
            }
            else {
                auto maybeModel = resolveFallbackModelID(entryKey, debugBuffer);
                if (maybeModel.has_value()) {
                    model = maybeModel.value();
                }
                else { continue; }
            }
        }
        else {
            auto maybeModel = resolveFallbackModelID(entryKey, debugBuffer, true);
            if (maybeModel.has_value()) {
                model = maybeModel.value();
            }
            else { continue; }
        }

        SimObjects[entryKey] = new simulationObject(shader, model);

        if (entryValue.contains("color")) {
            auto color = Color(entryValue["color"].get<std::string>());
            SimObjects[entryKey]->model->objectColor = glm::vec3( color.decR, color.decG, color.decB );
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
                auto lightColor = Color(entryValue["light"]["color"].get<std::string>());
                color = glm::vec3( lightColor.decR, lightColor.decG, lightColor.decB );
            } else {
                color = SimObjects[entryKey]->model->objectColor;
            }

            SimObjects[entryKey]->light = new LightObject(SimObjects[entryKey]->position, color, intensity);            
        }
        else if (SimObjects[entryKey]->objectType == "star") {
            SimObjects[entryKey]->light = new LightObject();
        }
    }

    std::string debugOutputString = debugBuffer.str();
    bool debugNecesery = debugOutputString.empty();

    if (debugMode) {
        std::cout << (debugNecesery ? formatSuccess("Done") : formatWarning("Done with exceptions")) << (debugNecesery ? "" : "\n" + debugOutputString) << std::endl;
    }

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
                if (debugMode) { debugBuffer << formatError("ERROR") << ": In scene '" << formatPath(sceneID) << "' and object instance not specified ... " << formatWarning("Skipping") << '\n'; }
                continue;
            }

            SimObjectID objectID = simObject["object"].get<std::string>();
            if (SimObjects.find(objectID) == SimObjects.end()) {
                if (debugMode) { debugBuffer << formatError("ERROR") << ": Object '" << formatPath(objectID) << "' in scene '" << formatPath(sceneID) << "' ... " << formatWarning("Skipping") << '\n'; }
                continue;
            }
            
            simulationObject* currentSimObject = SimObjects[objectID];

            if (simObject.contains("position")) {
                if (simObject["position"].size() == 3) {
                    auto position = simObject["position"];
                    currentSimObject->realPosition = glm::vec3(position[0].get<double>(), position[1].get<double>(), position[2].get<double>());
                }
                else {
                    if (debugMode) { debugBuffer << formatWarning("WARNING") << ": In scene '" << formatPath(sceneID) << "' position has incorrect format " << formatProcess("[x, y, z]") << " for object '" << formatPath(objectID) << "' ... " << formatProcess("Loading defaults") << '\n'; }
                    currentSimObject->realPosition = glm::dvec3(0);
                }
            }
            else {
                if (debugMode) { debugBuffer << formatWarning("WARNING") << ": In scene '" << formatPath(sceneID) << "' position not found for object '" << formatPath(objectID) << "' ... " << formatProcess("Loading defaults") << '\n'; }
                currentSimObject->position = glm::dvec3(0);
            }

            if (simObject.contains("velocity")) {
                if (simObject["velocity"].size() == 3) {
                    auto position = simObject["velocity"];
                    currentSimObject->realVelocity = glm::dvec3(position[0].get<double>(), position[1].get<double>(), position[2].get<double>());
                }
                else {
                    if (debugMode) { debugBuffer << formatWarning("WARNING") << ": In scene '" << formatPath(sceneID) << "' velocity has incorrect format " << formatProcess("[x, y, z]") << " for object '" << formatPath(objectID) << "' ... " << formatProcess("Loading defaults") << '\n'; }
                    currentSimObject->velocity = glm::dvec3(0);
                }
            }
            else {
                if (debugMode) { debugBuffer << formatWarning("WARNING") << ": In scene '" << formatPath(sceneID) << "' velocity not found for object '" << formatPath(objectID) << "' ... " << formatProcess("Loading defaults") << '\n'; }
                currentSimObject->velocity = glm::dvec3(0);
            }

            currentScene.insert(currentSimObject);
        }

        if (currentScene.empty()) {
            debugBuffer << formatError("ERROR") << ": In scene '" << formatPath(sceneID) << "' no objects were found or passed tests ... " << formatError("Skipping") << '\n';
        }
        else {
            Scenes::allScenes[sceneID] = currentScene;
            currentScene.clear(); // likely not necesery but just to be sure
        }
        
    }

    std::string debugOutputString = debugBuffer.str();
    bool debugNecesery = debugOutputString.empty();

    if (debugMode) {
        std::cout << (debugNecesery ? formatSuccess("Done") : formatWarning("Done with exceptions")) << (debugNecesery ? "" : "\n" + debugOutputString) << std::endl;
    }

    // setting up default fallback scene
    if (Scenes::allScenes.empty()) {
        if (debugMode) { std::cout << formatWarning("WARNING") << ": No scenes were loaded / found" << std::endl; }
    }
    else {
        switchSceneAndCalculateObjects( (*Scenes::allScenes.begin()).first );
    }

}