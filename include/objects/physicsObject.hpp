#ifndef PHYSICS_OBJECT_HEADER
#define PHYSICS_OBJECT_HEADER

#include <types.hpp>
#include <stlImport.hpp>

#include <vector>

class physicsObject {
    public:
        mutable glm::dvec3 position = glm::dvec3(0);

        mutable units::kilometers radius;
        mutable units::tons mass;

        mutable glm::dvec3 movementVector = glm::dvec3(0);
};

#endif // PHYSICS_OBJECT_HEADER