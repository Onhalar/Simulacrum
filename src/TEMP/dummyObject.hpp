#include <config.hpp>
#include <globals.hpp>

#include <VBO.hpp>
#include <EBO.hpp>
#include <VAO.hpp>

class dummyObject {
    public:
        dummyObject() {

            GLfloat vertices[] = {
                // vertex position      vertex color
                -0.5f, -0.5f, 0.0f,     1.0f, 0.0f, 0.0f,
                0.5f, -0.5f, 0.0f,      0.0f, 1.0f, 0.0f,
                0.0f,  0.5f, 0.0f,      0.0f, 0.0f, 1.0f,
            
                // inner vertices
                0.0f, -0.5f, 0.0f,      1.0f, 0.0f, 0.0f,
                0.25f, 0.0f, 0.0f,      0.0f, 1.0f, 0.0f,
                -0.25f, 0.0f, 0.0f,     0.0f, 0.0f, 1.0f,
            };

            GLuint indices[] = {
                5, 4, 2, // upper triangle
                0, 3, 5, // lower left triangle
                3, 1, 4 // upper right triangle
            };

            VAO1.bind();

            VBO VBO1(vertices, sizeof(vertices));
            EBO EBO1(indices, sizeof(indices));

            VAO1.linkAttrib(VBO1, 0, 3, GL_FLOAT, 6 * sizeof(float), (void*)0);
            VAO1.linkAttrib(VBO1, 1, 3, GL_FLOAT, 6 * sizeof(float), (void*)(3 * sizeof(float)));

            VAO1.unbind();
            VBO1.unbind();
            EBO1.unbind();

            GLuint uniID = glGetUniformLocation(mainShader->ID, "scale");

            mainShader->activate();

            glUniform1f(uniID, 0.5f);
        }

        void draw() {
            VAO1.bind();
            glDrawElements(GL_TRIANGLES, 9, GL_UNSIGNED_INT, 0);
        }
    private:
        VAO VAO1;
};