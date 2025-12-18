#ifndef FULL_SIMULATION_OBJECT_HEADER
#define FULL_SIMULATION_OBJECT_HEADER

#include <string>
#include <unordered_map>

#include <model.hpp>
#include <shader.hpp>

#include <lightObject.hpp>

#include <globals.hpp>

class simulationObject {
    private:
        ShaderID shaderID;
        ModelID modelId;

        // original (backup settings) - only the physics one that can at all change

        glm::dvec3 originalPosition = glm::dvec3(0.0f);
        glm::dvec3 originalvelocity = glm::dvec3(0.0);
    public:
        // vertex position - scaled
        glm::vec3 vertPosition = glm::vec3(0.0f);

        // real-world position - km
        glm::dvec3 position = glm::dvec3(0.0f);

        units::kilometers radius = -1.0;
        units::tons mass = -1.01;

        // real-world velocity - km/s
        glm::dvec3 velocity = glm::dvec3(0.0);

        std::string name;

        float vertexModelRadius = -1.0; // -1 is imposible since output is an absolute number
        float vertexRotation = 0.0; // degrees / second

        double rotationSpeed = -1; // km/h

        mutable Shader* shader;
        mutable Model* model;

        glm::mat4 modelMatrix = glm::mat4(1);
        std::string objectType = "planet";

        LightObject* light = nullptr;

        simulationObject(ShaderID shaderID, ModelID modelID) {
            shader = Shaders[shaderID];
            model = new Model(*Models[modelID]);
        }
        simulationObject(const simulationObject& original) {
            this->shader = original.shader;
            this->model = original.model;

            this->mass = original.mass;
            this->light = original.light;
            this->objectType = original.objectType;

            this->position = original.position;
            this->velocity = original.velocity;

            this->modelMatrix = original.modelMatrix;
            this->vertexModelRadius = original.vertexModelRadius;
            this->position = original.position;
            this->radius = original.radius;
            this->rotationSpeed = original.rotationSpeed;
            this->vertexRotation = original.vertexRotation;

            this->modelId = original.modelId;
            this->shaderID = original.shaderID;

            this->name = original.name;
        }

        ~simulationObject() {} // shader and model class instances will get deleted in the cleanup loop.

        // loads original physics values
        void loadOriginalValues() {
            position = originalPosition;
            velocity = originalvelocity;
        }
        // sets current physics values as original
        void setCurrentAsOriginal() {
            originalPosition = position;
            originalvelocity = velocity;
        }

        void calculateAproximateRadius() {

            auto vertices = model->modelData.vertices;
            // vertices passed will be render-ready so x, y, z, x, y, z, ...

            float minX = vertices[0]; float MaxX = vertices[0];
            float minY = vertices[1]; float MaxY = vertices[1];
            float minZ = vertices[2]; float MaxZ = vertices[2];

            for (auto vertex = vertices.begin() + 3; /* 3 items per vertex; x, y, z*/ vertex != vertices.end(); vertex += 3) {
                minX = std::min( *vertex, minX);
                MaxX = std::max( *vertex, MaxX);

                minY = std::min( *(vertex + 1), minY);
                MaxY = std::max( *(vertex + 1), MaxY);

                minZ = std::min( *(vertex + 2), minZ);
                MaxZ = std::max( *(vertex + 2), MaxZ);
            }

            float DifferenceX = std::abs(MaxX - minX);
            float DifferenceY = std::abs(MaxY - minY);
            float DifferenceZ = std::abs(MaxZ - minZ);

            vertexModelRadius = std::max(DifferenceX, std::max(DifferenceY, DifferenceZ)) * 0.5f;
        }
        
        void normalizeVertices(const float normalizedRadius) {
            if (vertexModelRadius == -1) { 
                if (debugMode) { std::cerr << formatError("ERROR") << ": Normalization of " << name << "'s vertiecs attempted before calculating radius - ABORTED" << std::endl; }

                return;
            }

            float scalingFactor = normalizedRadius / vertexModelRadius;

            scaleVertices(scalingFactor);

            vertexModelRadius = normalizedRadius;
        }

        void scaleVertices(const float& scaleFactor) {
            for (auto& verticeAxee : model->modelData.vertices) {
                verticeAxee *= scaleFactor;
            }
        }

        void draw() {
            model->draw(shader);
        }

};

using SimObjectList = std::unordered_map<std::string, simulationObject*>;

SimObjectList SimObjects;

#endif // FULL_SIMULATION_OBJECT_HEADER