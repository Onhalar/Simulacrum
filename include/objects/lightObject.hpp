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
        mutable glm::vec4 lightColor;

        LightObject(Shader* shader, GLfloat* vertices, size_t verticesSize, GLuint* indices, size_t indicesSize, glm::vec4 lightColor = glm::vec4(1.0f)) {
            this->shader = shader;
            this->lightColor = lightColor;

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
        }

        void applyLightColor(glm::vec4 lightColor) {
            shader->setUniform("lightColor", glm::vec4(lightColor));
            this->lightColor = lightColor;
        }

        void draw() {
            shader->activate();

            shader->applyModelMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(0.8f, 0.45f, 0.0f)));

            VAO1.bind();

            glDrawElements(GL_TRIANGLES, amountOfVertices, GL_UNSIGNED_INT, 0);

            VAO1.unbind();
        }
    private:
        VAO VAO1;
        Shader* shader;
        unsigned int amountOfVertices;
};
