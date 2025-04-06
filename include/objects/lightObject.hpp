#include <VBO.hpp>
#include <EBO.hpp>
#include <VAO.hpp>
#include <texture.hpp>

#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

class LightObject {
    public:
        glm::vec4 lightColor;
        glm::vec3 lightPosition;
        GLfloat lightIntensity;
        GLfloat lightDistance;

        LightObject(Shader* shader, GLfloat* vertices, size_t verticesSize, GLuint* indices, size_t indicesSize, glm::vec4 lightColor = glm::vec4(1.0f), glm::vec3 lightPosition = glm::vec3(0.0f), GLfloat intensity = 1.0f, float lightDistance = 0.5f) {
            this->shader = shader;
            this->lightColor = lightColor;
            this->lightPosition = lightPosition;
            this->lightIntensity = intensity;
            this->lightDistance = lightDistance;

            amountOfVertices = indicesSize;

            VAO1.bind();

            VBO VBO1(vertices, sizeof(float) * verticesSize);
            EBO EBO1(indices, sizeof(int) * indicesSize);

            VAO1.linkAttrib(VBO1, 0, 3, GL_FLOAT, 3 * sizeof(float), (void*)0);

            VAO1.unbind();
            VBO1.unbind();
            EBO1.unbind();

            shader->activate();

            applyLightColor(lightColor);

            glm::mat4 modelMatrix = shader->modelMatrix = glm::translate(glm::mat4(1.0f), lightPosition);

            shader->applyModelMatrix();

            updatePosition();
        }

        void applyLightColor(glm::vec4 lightColor) {
            shader->setUniform("lightColor", glm::vec4(lightColor));
            this->lightColor = lightColor;
        }
        // will be important when having multiple light sources in scene
        void applyLightColor() {
            shader->setUniform("lightColor", glm::vec4(lightColor));
        }

        void updatePosition(glm::vec3 newPosition) {
            lightPosition = newPosition;
            glm::mat4 modelMatrix = shader->modelMatrix = glm::translate(glm::mat4(1.0f), lightPosition);
            
            if (shader->modelMatrix != modelMatrix) {
                shader->modelMatrix = modelMatrix;
                shader->applyModelMatrix();
            }
        }
        void updatePosition() {
            glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), lightPosition);
            
            if (shader->modelMatrix != modelMatrix) {
                shader->modelMatrix = modelMatrix;
                shader->applyModelMatrix();
            }
        }

        void draw() {
            shader->activate();

            VAO1.bind();

            glDrawElements(GL_TRIANGLES, amountOfVertices, GL_UNSIGNED_INT, 0);

            VAO1.unbind();
        }
    private:
        VAO VAO1;
        Shader* shader;
        unsigned int amountOfVertices;
};
