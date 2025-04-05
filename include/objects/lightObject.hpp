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
        LightObject(Shader* shader, GLfloat* vertices, size_t verticesSize, GLuint* indices, size_t indicesSize) {
            this->shader = shader;

            amountOfVertices = indicesSize;

            VAO1.bind();

            VBO VBO1(vertices, sizeof(float) * verticesSize);
            EBO EBO1(indices, sizeof(int) * indicesSize);

            VAO1.linkAttrib(VBO1, 0, 3, GL_FLOAT, 3 * sizeof(float), (void*)0);

            VAO1.unbind();
            VBO1.unbind();
            EBO1.unbind();
        }

        void draw() {
            shader->activate();

            shader->applyModelMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(0.45f, 0.45f, 0.45f)));

            VAO1.bind();

            glDrawElements(GL_TRIANGLES, amountOfVertices, GL_UNSIGNED_INT, 0);

            VAO1.unbind();
        }
    private:
        VAO VAO1;
        Shader* shader;
        unsigned int amountOfVertices;
};
