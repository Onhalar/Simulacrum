#include "globals.hpp"
#include "json.hpp"
#include "lightObject.hpp"
#include <algorithm>
#include <exception>
#include <filesystem>
#include <format>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
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

#include <stdexcept>
#include <functional>


using Json = nlohmann::json;
using errorCode = std::string;

void loadSimObjects(std::filesystem::path path);
void loadPhysicsScene(std::filesystem::path path);

void setupSimulation() {
    loadSimObjects(projectPath(simObjectsConfigPath));

    loadPhysicsScene(projectPath(physicsScenesPath));
}



// attempts to assign a value from json data to given variable / member; if default value is provided it will disregard missing values from json - with custom assignment logic
template <typename dest, typename src>
inline void assignValue(const std::string& objectName, dest& objectValue, const Json& jsonData, const std::string& jsonKey, std::function<void(dest& destination, const src& source)> action, const std::optional<src> defaultValue = std::nullopt, std::stringstream* const debugBuffer = nullptr) {    
    src value;
    
    if (!jsonData.contains(jsonKey)) {
        if (!defaultValue.has_value()) {
            throw std::invalid_argument( std::format("Could not find property '{}' in loaded configuration for object '{}'", jsonKey, objectName) );
        }
        else {
            if (debugBuffer && debugMode) {
                *debugBuffer << formatWarning("WARNING") << ": could not find '" << colorText(jsonKey, ANSII_MAGENTA) << "' in the config of '" << colorText(objectName, ANSII_MAGENTA) << "' object ... " << formatProcess("Loading defaults") << "\n";
            }
            value = defaultValue.value();
        }
    }
    else {
        value = jsonData[jsonKey].get<src>();
    }
    
    action(objectValue, value);
}
// attempts to assign a value from json data to given variable / member; if default value is provided it will disregard missing values from json
template <typename src, typename dest>
inline void assignValue(const std::string& objectName, dest& objectValue, const Json& jsonData, const std::string& jsonKey, const std::optional<src> defaultValue = std::nullopt, std::stringstream* const debugBuffer = nullptr) {    
    src value;
    
    if (!jsonData.contains(jsonKey)) {
        if (!defaultValue.has_value()) {
            throw std::invalid_argument( std::format("Could not find property '{}' in loaded configuration for object '{}'", jsonKey, objectName) );
        }
        else {
            if (debugBuffer && debugMode) {
                *debugBuffer << formatWarning("WARNING") << ": could not find '" << colorText(jsonKey, ANSII_MAGENTA) << "' in the config of '" << colorText(objectName, ANSII_MAGENTA) << "' object ... " << formatProcess("Loading defaults") << "\n";
            }
            value = defaultValue.value();
        }
    }
    else {
        value = jsonData[jsonKey].get<src>();
    }

    objectValue = value;
}


Json loadJsonData(std::filesystem::path filePath) {
    if ( !std::filesystem::exists(filePath) || !std::filesystem::is_regular_file(filePath) ) {
        throw std::invalid_argument( std::format("Could not open and load Json data from '{}'", formatPath(filePath.string())) );
    }

    std::ifstream file(filePath);
    Json data;
    file >> data;
    file.close();

    return data;
}


void handleDebugBuffer(std::stringstream& buffer) {
    if (!debugMode) { return; }
    std::string bufferContents = buffer.str();
    bool debugPresent = !bufferContents.empty();

    if (debugPresent) {
        std::cout << formatWarning("Done with exceptions") << "\n" << bufferContents << std::endl;
    }
    else {
        std::cout << formatSuccess("Done") << std::endl;
    }
}


// -----------------===[ Calculating Values ]===-----------------



// retrieves the object with the largest mass
simulationObject* getGravityWhell(const std::vector<SimObjectID> objectIDs) {
    std::pair<units::tons, simulationObject*> largestMass = {0.0, nullptr};

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
    if (object->position == gravityWhell->position) { return velocity; }
    
    units::kilometers distance = glm::distance(gravityWhell->position, object->position);
    
    double neededVelocityMpS = std::sqrt((GRAVITATIONAL_CONSTANT * /*Tons*/ gravityWhell->mass.get<units::kilograms>()) / (double)distance.get<units::meters>());
    double neededVelocityKpS = neededVelocityMpS / 1000.0;

    velocity = orbitalVector * neededVelocityKpS;

    return velocity;
}



// -----------------===[ Helper Functions ]===-----------------



void assignColor(glm::vec3& dest, const std::string& src) {
    Color color = Color(src);
    dest = {color.decR, color.decG, color.decB};
}



// -----------------===[ Import Handlers ]===-----------------



void loadSimObjects(std::filesystem::path path) {
    Json data;

    try {
        if (debugMode) { std::cout << formatProcess("\nLoading") << " objects from '" << formatPath(path.filename().string()) << "' ... "; }
        data = loadJsonData(path);
    }
    catch (std::exception e) {
        if (debugMode) { std::cerr << formatError("FAILED") << "\n" << formatError("ERROR") << ": " << e.what(); }
        return;
    }


    std::stringstream debugBuffer;

    static ModelID fallbackModel = (Models.empty() ? "" : Models.begin()->first);
    static std::optional<std::string> fallbackColor = "#ff00ff";
    static std::optional<double> earthFallbackRotationSpeed = 0.003992; //360 * ((earthRotationKmH / (EarthRadius*PI*2)) / 3600) -> approximate Earth's rotation degrees / second
    static std::optional<std::string> fallbackObjectType = "planet";



    for (const auto& [objectID, object] : data.items()) {

        ShaderID shader;
        ModelID model;

        // Shader
        if (object.contains("shader")) {
            if (Shaders.contains(object["shader"])) {
                  shader = object["shader"];
            }
            else {
                debugBuffer << formatError("ERROR") << ": cannot load invalid shader '" << colorText(object["shader"], ANSII_MAGENTA) << "' for object '" << objectID << "' ... " << formatProcess("skipping") << std::endl;
                continue;
            }
        }
        else {
            debugBuffer << formatError("ERROR") << ": shader '" << colorText(object["shader"], ANSII_MAGENTA) << "' does't exist - in object '" << objectID << "' ... " << formatProcess("skipping") << std::endl;
            continue;
        }

        // Model
        if (object.contains("model")) {
            if (Models.contains(object["model"])) {
                model = object["model"];
            }
            else {
                debugBuffer << formatError("ERROR") << ": cannot load invalid model '" << colorText(object["model"], ANSII_MAGENTA) << "' ... ";
                if (fallbackModel != "") {
                    model = fallbackModel; debugBuffer << formatSuccess("Done") << std::endl;
                }
                else {
                    debugBuffer << formatError("FAILED") << " ... " << formatProcess("skipping") << std::endl;
                    continue;
                }
            }
        }
        else {
            debugBuffer << formatError("ERROR") << ": shader '" << colorText(object["model"], ANSII_MAGENTA) << "' does't exist - in object '" << objectID << "' ... " << formatProcess("Loading defaults") << " ... ";
            if (fallbackModel != "") {
                model = fallbackModel; std:: cout << formatSuccess("Done") << std::endl;
            }
            else {
                std::cout << formatError("FAILED") << " ... " << formatProcess("skipping") << std::endl;
                continue;
            }
        }
        

        simulationObject* simObject = new simulationObject(shader, model, true /*derive model*/ );


        // process the rest

        simObject->name = objectID;
        assignValue<double>(objectID, simObject->radius, object, "radius");
        assignValue<double>(objectID, simObject->mass, object, "mass");
        assignValue<double>(objectID, simObject->radius, object, "radius");
        assignValue<glm::vec3, std::string>(objectID, simObject->model->color, object, "color", assignColor, fallbackColor, &debugBuffer);
        assignValue<std::string>(objectID, simObject->objectType, object, "type", fallbackObjectType, &debugBuffer);
        assignValue<double>(objectID, simObject->rotationSpeed, object, "rotation", earthFallbackRotationSpeed, &debugBuffer);
        
        // Light
        if (simObject->objectType == "star") {
            if (!object.contains("light")) {
                simObject->light = new LightObject;
            }
            else {
                assignValue<LightObject*, Json>(objectID, simObject->light, object, "light",
                    [&](LightObject*& obj, const Json& value) {
                        obj = new LightObject();
                        assignValue<float>(objectID, obj->intensity, value, "intensity");
                        assignValue<glm::vec3, std::string>(objectID, obj->color, value, "color", assignColor, fallbackColor, &debugBuffer);
                        assignValue<char, Json>(objectID, obj->starType, value, "starType", [&](char& obj, const Json& value) {obj = value.get<std::string>()[0]; });
                    }
                );
            }
        }

        SimObjects[objectID] = simObject;
    }

    handleDebugBuffer(debugBuffer);
}





void loadPhysicsScene(std::filesystem::path path) {
    Json data;

    try {
        if (debugMode) { std::cout << formatProcess("\nLoading") << " objects from '" << formatPath(path.filename().string()) << "' ... "; }
        data = loadJsonData(path);
    }
    catch (std::exception e) {
        if (debugMode) { std::cerr << formatError("FAILED") << "\n" << formatError("ERROR") << ": " << e.what(); }
        return;
    }


    glm::vec3 orbitVector(0.0f, 1.0f, 0.0f);
    std::stringstream debugBuffer;

    if (data.contains("ORBIT")) {
        auto ORBIT = data["ORBIT"];
        orbitVector = glm::vec3(
          ORBIT[0].get<float>(),
          ORBIT[1].get<float>(),
          ORBIT[2].get<float>()  
        );
    }


    
    for (const auto& [sceneID, sceneData] : data.items()) {
        if (sceneID == "ORBIT") { continue; }

        std::vector<SimObjectID> objectIDs;
        std::unordered_map<SimObjectID, simulationObject*> objectCache;
        

        scene* currentScene = new scene();


        // gather object IDs beforehand for gravity whell calculations
        for (const auto& simObject: sceneData["objects"]) {
            if (!simObject.contains("object")) { continue; }
            objectIDs.push_back(simObject["object"].get<std::string>());
        }

        simulationObject* gravityWhell = getGravityWhell(objectIDs);


        // --- OBJECTS --
        if (!sceneData.contains("objects")) {
            if (debugMode) { debugBuffer << formatError("ERROR") << ": could not find objects in scene '" << colorText(sceneID, ANSII_MAGENTA) << "' ... skipping\n"; }
            continue;
        }
        for (const auto& objectData : sceneData["objects"]) {

            SimObjectID objectID;
            assignValue<SimObjectID>("", objectID, objectData, "object");
            
            simulationObject* simObject = new simulationObject(*SimObjects[objectID], true); // creates a derived model


            assignValue<glm::dvec3, Json>(objectID, simObject->position, objectData, "position", 
                [&](glm::dvec3& dest, const Json& src) {
                    dest = {
                        src[0].get<double>(),
                        src[1].get<double>(),
                        src[2].get<double>()
                    };
                }
            );

            // ToDO: clean up somehow
            glm::dvec3 idealVelocity = calcIdealOrbitVelocity(simObject, gravityWhell, orbitVector);
            assignValue<glm::dvec3, Json>(objectID, simObject->velocity, objectData, "velocity", 
                [&](glm::dvec3& dest, const Json& src) {
                    dest = {
                        src[0].get<double>(),
                        src[1].get<double>(),
                        src[2].get<double>()
                    };
                },
                (std::optional<Json>)(Json){idealVelocity.x, idealVelocity.y, idealVelocity.z},
                &debugBuffer
            );

            simObject->setCurrentAsOriginal();

            objectCache[objectID] = simObject;
            currentScene->objects.push_back(simObject);
        }


        // --- GROUPS ---
        if (!sceneData.contains("groups")) {
            if (debugMode) { debugBuffer << formatError("ERROR") << ": could not find sim groups in scene '" << colorText(sceneID, ANSII_MAGENTA) << "' ... skipping\n"; }
            continue;
        }
        for (const auto& group : sceneData["groups"]) {
            sceneGroup currentGroup;

            for (const auto& member : group) {
                currentGroup.push_back(objectCache[member.get<std::string>()]);
            }

            currentScene->groups.push_back(currentGroup);
        }

        // sort by distance from origin
        std::sort(currentScene->objects.begin(), currentScene->objects.end(), 
            [](simulationObject* a, simulationObject* b){
                static glm::dvec3 origin = {0.0, 0.0, 0.0};
                if (a->position == b->position) { return true; }
                double distA = glm::distance(origin, a->position), distB = glm::distance(origin, a->position);

                return distA > distB;
            }
        );

        Scenes::allScenes[sceneID] = currentScene;
    }
    
    handleDebugBuffer(debugBuffer);
}
