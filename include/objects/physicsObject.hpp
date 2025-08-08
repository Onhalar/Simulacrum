#ifndef PHYSICS_OBJECT_HEADER
#define PHYSICS_OBJECT_HEADER

#include <types.hpp>

class physicsObject {
    public:
        mutable glm::vec3 position = glm::vec3(0);

        mutable units::kilometers radius = -1.0;
        mutable units::tons mass = -1.01;

        mutable glm::dvec3 velocity = glm::dvec3(0);
};

#endif // PHYSICS_OBJECT_HEADER