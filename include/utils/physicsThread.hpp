#ifndef PHYSICS_THREAD_HEADER
#define PHYSICS_THREAD_HEADER

#include "scenes.hpp"
#include "simObject.hpp"
#include "types.hpp"
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <unordered_map>
#include <utility>
#include <vector>


#include <glm/glm.hpp>
#include <units.hpp>

inline std::mutex physicsMutex;
inline std::condition_variable physicsCV;
inline std::atomic<bool> physicsRunning(true);
inline std::atomic<bool> pausePhysicsThread(false);

inline double physicsDeltaTime;

inline std::thread physicsThread;

struct SnapObj {
    glm::dvec3 position;
    glm::dvec3 velocity;
    glm::dvec3 acceleration;
    units::tons mass;

    glm::dvec3 vertPosition;
    double distanceScale;
    bool simulate;
    bool firstPass;
};

class Snapshot {
    public:
        std::vector<std::vector<SnapObj*>> groups;
        std::vector<std::pair<SnapObj*, simulationObject*>> objects;
        SceneID ID;

    public:

        void takeSnapshot(bool lock = true) {
            bool takeFullSnapshot = Scenes::currentSceneID != ID;

            if (lock) { physicsMutex.lock(); }

            if (takeFullSnapshot) { fullSnapshot(Scenes::currentScene); }
            else { lightSnapshot(Scenes::currentScene); }

            if(lock) { physicsMutex.unlock(); }
        }

        void updateOrigin(bool lock = true) {
            const scene* scene = Scenes::currentScene;

            if (lock) { physicsMutex.lock(); }

            for ( auto [snapObj, obj] : objects ) {
                obj->position = snapObj->position;
                obj->velocity = snapObj->velocity;
                obj->acceleration = snapObj->acceleration;
                obj->vertPosition = snapObj->vertPosition;

                obj->firstPass = snapObj->firstPass;
            }

            if (lock) { physicsMutex.unlock(); }
        }

        ~Snapshot() {
            clearData();
        }

    private:

        void clearData() {
            groups.clear();

            for (auto obj : objects) {
                if (!obj.first) { continue; }
                delete obj.first;
                obj.first = nullptr;
            }
            objects.clear();
        }

        void fullSnapshot(scene* scene) {
            std::unordered_map<std::string, SnapObj*> dict;

            groups.clear();
            dict.clear();
            ID = Scenes::currentSceneID;

            for (const auto obj : scene->objects) {
                SnapObj* newObj = new SnapObj(
                    obj->position,
                    obj->velocity,
                    obj->acceleration,
                    obj->mass,

                    obj->vertPosition,
                    obj->distanceScale,
                    obj->simulate,
                    obj->firstPass
                );

                dict[obj->name] = newObj;
                objects.push_back({ newObj, obj });
            }

            groups.reserve(scene->groups.size());
            for (const auto group : scene->groups) {
                std::vector<SnapObj*> currentGroup;
                currentGroup.reserve(group.size());

                for (const auto obj : group) {
                    currentGroup.push_back(dict[obj->name]);
                }

                groups.push_back(currentGroup);
            }

            dict.clear();
        }

        void lightSnapshot(scene* scene) {
            for ( auto [snapObj, obj] : objects ) {
                snapObj->position = obj->position;
                snapObj->velocity = obj->velocity;
                snapObj->acceleration = obj->acceleration;

                snapObj->firstPass = obj->firstPass;
            }
        }
};

#endif // PHYSICS_THREAD_HEADER