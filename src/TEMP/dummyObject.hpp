#include <config.hpp>
#include <globals.hpp>

#include <VBO.hpp>
#include <EBO.hpp>
#include <VAO.hpp>
#include <texture.hpp>

class dummyObject {
    public:
        dummyObject() {
            Texture example(projectPath("res/img/example.png").c_str(), GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE);
            example.textureUnit(mainShader, "texture0", 0);
            example.bind();

            GLfloat vertices[] = {
                // vertex position        vertex color              texture coordinates
                -0.5f,  -0.5f, 0.0f,        1.0f, 0.0f, 0.0f,         0.0f, 0.0f,
                -0.5f, 0.5f, 0.0f,          0.0f, 1.0f, 0.0f,         0.0f, 1.0f,
                0.5f, 0.5f, 0.0f,           0.0f, 0.0f, 1.0f,         1.0f, 1.0f,
                0.5f, -0.5f, 0.0f,          0.5f, 1.0f, 0.0f,         1.0f, 0.0f
            };

            GLuint indices[] = {
                0, 2, 1,
                0, 3, 2
            };

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
            VAO1.bind();
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
    private:
        VAO VAO1;
};
