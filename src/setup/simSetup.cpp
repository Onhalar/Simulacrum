#include <units.hpp>
#include <config.hpp>
#include <types.hpp>
#include <renderDefinitions.hpp>

#include <color.hpp>
#include <simObject.hpp>
#include <scenes.hpp>

#include <debug.hpp>
#include <FormatConsole.hpp>
#include <paths.hpp>

#include <unordered_map>
#include <unordered_set>
#include <optional>

#include <stdexcept>


using Json = nlohmann::json;
using errorCode = std::string;

void loadSimObjects(std::filesystem::path path);
void loadPhysicsScene(std::filesystem::path path);

void setupSimulation() {
    loadSimObjects(projectPath(simObjectsConfigPath));

    loadPhysicsScene(projectPath(physicsScenesPath));
}

// retrieves the object with the largest mass
simulationObject* getGravityWhell(const std::vector<SimObjectID> objectIDs) {
    std::pair<units::tons, simulationObject*> largestMass = {DBL_MAX, nullptr};

    for (const SimObjectID&  ID : objectIDs) {
        simulationObject* currentObject = SimObjects[ID];
        if (!largestMass.second) {
            largestMass = {currentObject->mass, currentObject};
            continue;
        }
        
        if (currentObject->mass > largestMass.second->mass) { largestMass = {currentObject->mass, currentObject}; }
    }

    return largestMass.second;
}

// calculates the ideal orbital velocity of a body.
glm::dvec3 calcIdealOrbitVelocity(const simulationObject* object, const simulationObject* gravityWhell, const glm::dvec3& orbitalVector /*orbital velocity vector*/) {
    glm::dvec3 velocity(0.0);
    if (!object) { throw std::invalid_argument("Object does not exis - calc orbital velocity"); }
    else if (!gravityWhell) { throw std::invalid_argument("Gravity whell does not exis - calc orbiral velocity"); }
    if (object->realPosition == gravityWhell->realPosition) { return velocity; }
    
    units::kilometers distance = glm::distance(gravityWhell->realPosition, object->realPosition);
    
    double neededVelocityMpS = std::sqrt((GRAVITATIONAL_CONSTANT * /*Tons*/ gravityWhell->mass.get<units::kilograms>()) / (double)distance.get<units::meters>());
    double neededVelocityKpS = neededVelocityMpS / 1000.0;

    velocity = orbitalVector * neededVelocityKpS;

    printVec3("velocity", velocity);

    return velocity;
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

        SimObjects[entryKey]->name = entryKey;

        if (entryValue.contains("color")) {
            auto color = Color(entryValue["color"].get<std::string>());
            SimObjects[entryKey]->model->objectColor = glm::vec3( color.decR, color.decG, color.decB );
        }

        if (entryValue.contains("radius")) {
            SimObjects[entryKey]->radius = (units::kilometers)entryValue["radius"].get<double>();
        }

        if (entryValue.contains("mass")) {
            SimObjects[entryKey]->mass = (units::tons)entryValue["mass"].get<double>();
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
    // the direction of standard orbit
    glm::vec3 orbitVector(0.0f, 1.0f, 0.0f);

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

    if (data.contains("ORBIT")) {
        auto ORBIT = data["ORBIT"];
        orbitVector = glm::vec3(
          ORBIT[0].get<float>(),
          ORBIT[1].get<float>(),
          ORBIT[2].get<float>()  
        );
    }
    
    for (const auto& entry : data.items()) {

        auto sceneID = entry.key();
        auto entryValue = entry.value();


        if (sceneID == "ORBIT") { continue; }


        std::vector<SimObjectID> objectIDs = {};

        // gather object IDs for future use; I will not be updating the following loop because hell nah.
        for (const auto& simObject: entryValue["objects"]) {
            if (!simObject.contains("object")) { continue; }
            objectIDs.push_back(simObject["object"].get<std::string>());
        }  

        static simulationObject* gravityWhell = getGravityWhell(objectIDs);

        std::unordered_map<std::string, simulationObject*> objectCache;

        scene* currentScene = new scene();

        // ToDo: add checking clause
        for (const auto& simObject : entryValue["objects"]) {
            
            if (!simObject.contains("object")) {
                if (debugMode) { debugBuffer << formatError("ERROR") << ": In scene '" << formatPath(sceneID) << "' and object instance not specified ... " << formatWarning("Skipping") << '\n'; }
                continue;
            }

            SimObjectID objectID = simObject["object"].get<std::string>();
            if (SimObjects.find(objectID) == SimObjects.end()) {
                if (debugMode) { debugBuffer << formatError("ERROR") << ": Object '" << formatPath(objectID) << "' in scene '" << formatPath(sceneID) << "' ... " << formatWarning("Skipping") << '\n'; }
                continue;
            }
            
            simulationObject* currentSimObject = new simulationObject(*SimObjects[objectID]);
            objectCache[objectID] = currentSimObject;

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
                if (debugMode) { debugBuffer << formatWarning("WARNING") << ": In scene '" << formatPath(sceneID) << "' velocity not found for object '" << formatPath(objectID) << "' ... " << formatProcess("Loading ideal values.") << '\n'; }
                currentSimObject->realVelocity = calcIdealOrbitVelocity(currentSimObject, gravityWhell, orbitVector);
            }

            currentScene->objects.insert(currentSimObject);
        }

        // setting up groups
        if (entryValue.contains("groups") && !entryValue["groups"].empty()){
            for (const auto& group : entryValue["groups"]) {
                std::unordered_set<simulationObject*> currentGroup;

                for (const auto& groupMember : group) {
                    if (objectCache.find(groupMember) != objectCache.end()) {
                        currentGroup.insert(objectCache[groupMember]);
                    }
                }

                if (!currentGroup.empty()) { currentScene->groups.push_back(currentGroup); }
            }
        }

        if (currentScene->groups.empty() && !objectCache.empty()) {
            std::unordered_set<simulationObject*> currentGroup;

            for (auto& [key, object] : objectCache) {
                currentGroup.insert(object);
            }

            currentScene->groups.push_back(currentGroup);
        }

        if (currentScene->objects.empty()) {
            debugBuffer << formatError("ERROR") << ": In scene '" << formatPath(sceneID) << "' no objects were found or passed tests ... " << formatError("Skipping") << '\n';
        }
        else {
            Scenes::allScenes[sceneID] = currentScene;
        }
        
    }

    std::string debugOutputString = debugBuffer.str();
    bool debugNecesery = debugOutputString.empty();

    if (debugMode) {
        std::cout << (debugNecesery ? formatSuccess("Done") : formatWarning("Done with exceptions")) << (debugNecesery ? "" : "\n" + debugOutputString) << std::endl;
    }

    // setting up default fallback scene
    if (Scenes::allScenes.empty() && debugMode) {
        std::cout << formatWarning("WARNING") << ": No scenes were loaded / found" << std::endl;
    }
    // depricated since the scenes will now go through scene picker
    /*else {
        switchSceneAndCalculateObjects( (*Scenes::allScenes.begin()).first );
    }*/

}