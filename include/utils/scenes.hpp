#ifndef PHYSICS_SCENE_CLASS_HEADER
#define PHYSICS_SCENE_CLASS_HEADER

#include <simObject.hpp>
#include <unordered_map>
#include <types.hpp>

#include <set>

using scene = std::set<simulationObject*>;
using sceneList = std::unordered_map<std::string, scene>;

class Scenes {
    public:
        inline static sceneList allScenes; // inline propperly initializes it for some reason
        inline static scene currentScene;

    static void switchScene(SceneID sceneID) {
        currentScene = allScenes[sceneID];
    }
};

#endif // PHYSICS_SCENE_CLASS_HEADER