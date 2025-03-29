#include <config.hpp>
#include <globals.hpp>

#include <VBO.hpp>
#include <EBO.hpp>
#include <VAO.hpp>
#include <texture.hpp>

class dummyObject {
    public:
        glm::mat4 objectModelMatrix = mainShader->modelMatrix;

        dummyObject(const char* textureFilepath = "res/img/defaultTexture.png", Shader* shader = mainShader) {
            this->shader = shader;

            Texture example(projectPath(textureFilepath).c_str(), GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE);
            example.textureUnit(mainShader, "texture0", 0);
            example.bind();

            GLfloat vertices[] = {
                // vertex position        vertex color              texture coordinates
                -0.5f,  0.0f,  0.5f,        0.83f, 0.70f, 0.44f,         0.0f, 0.0f,
                -0.5f,  0.0f, -0.5f,        0.86f, 0.70f, 0.44f,         5.0f, 0.0f,
                 0.5f,  0.0f, -0.5f,        0.83f, 0.70f, 0.44f,         0.0f, 0.0f,
                 0.5f,  0.0f,  0.5f,        0.83f, 0.70f, 0.44f,         5.0f, 0.0f,

                 0.0f,  0.8f,  0.0f,        0.92f, 0.86f, 0.76f,         2.5f, 5.0f
            };

            GLuint indices[] = {
                // Base
                0, 1, 2,
                0, 2, 3,
                // Sides
                0, 4, 1,
                1, 4, 2,
                2, 4, 3,
                3, 4, 0
            };

            amountOfVertices = sizeof(indices) / sizeof(GL_UNSIGNED_INT);

            VAO1.bind();

            VBO VBO1(vertices, sizeof(vertices));
            EBO EBO1(indices, sizeof(indices));

            VAO1.linkAttrib(VBO1, 0, 3, GL_FLOAT, 8 * sizeof(float), (void*)0);
            VAO1.linkAttrib(VBO1, 1, 3, GL_FLOAT, 8 * sizeof(float), (void*)(3 * sizeof(float)));
            VAO1.linkAttrib(VBO1, 2, 2, GL_FLOAT, 8 * sizeof(float), (void*)(6 * sizeof(float)));

            VAO1.unbind();
            VBO1.unbind();
            EBO1.unbind();

            GLuint uniID = glGetUniformLocation(mainShader->ID, "scale");

            glUniform1f(uniID, 0.5f);

        }

        void draw() {
            shader->applyModelMatrix(objectModelMatrix);

            VAO1.bind();

            glDrawElements(GL_TRIANGLES, amountOfVertices, GL_UNSIGNED_INT, 0);
        }
    private:
        VAO VAO1;
        Shader* shader;
        int amountOfVertices;
};
