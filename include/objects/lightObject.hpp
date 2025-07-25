#ifndef LIGHT_OBJECT_CLASS_HEADER
#define LIGHT_OBJECT_CLASS_HEADER

#include <glm/glm.hpp>

class LightObject {
    public:
        mutable glm::vec3 position;
        mutable glm::vec3 color;
        mutable float intensity;

        LightObject (
            glm::vec3 position = glm::vec3(0,0,0),
            glm::vec3 color = glm::vec3(1,0,0),
            float intensity = 1
        ) {
            this->position = position;
            this->color = color;
            this->intensity = intensity;
        }

        void updatePosition (glm::vec3 position) { this->position = position; }
        void updateColor (glm::vec3 color) { this->color = color; }
        void updateIntesity (float intensity) { this->intensity = intensity; }

        void updateAll (
            glm::vec3 position = glm::vec3(0,0,0),
            glm::vec3 color = glm::vec3(1,0,0),
            float intensity = 1
        ) {
            this->position = position;
            this->color = color;
            this->intensity = intensity;
        }
};

#endif