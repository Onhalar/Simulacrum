#include <config.hpp>
#include <globals.hpp>

#include <VBO.hpp>
#include <EBO.hpp>
#include <VAO.hpp>
#include <texture.hpp>

// still static will update when necesery
class object {
    public:
        glm::mat4 objectModelMatrix = mainShader->modelMatrix;

        object(const char* textureFilepath = "res/img/defaultTexture.png", Shader* shader = mainShader) {
            this->shader = shader;

            Texture example(projectPath(textureFilepath).c_str(), GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE);
            example.textureUnit(mainShader, "texture0", 0);
            example.bind();

            GLfloat vertices[] =
            { //     COORDINATES     /        COLORS          /    TexCoord   /        NORMALS       //
                -0.5f, 0.0f,  0.5f,     0.83f, 0.70f, 0.44f, 	 0.0f, 0.0f,      0.0f, -1.0f, 0.0f, // Bottom side
                -0.5f, 0.0f, -0.5f,     0.83f, 0.70f, 0.44f,	 0.0f, 5.0f,      0.0f, -1.0f, 0.0f, // Bottom side
                 0.5f, 0.0f, -0.5f,     0.83f, 0.70f, 0.44f,	 5.0f, 5.0f,      0.0f, -1.0f, 0.0f, // Bottom side
                 0.5f, 0.0f,  0.5f,     0.83f, 0.70f, 0.44f,	 5.0f, 0.0f,      0.0f, -1.0f, 0.0f, // Bottom side
            
                -0.5f, 0.0f,  0.5f,     0.83f, 0.70f, 0.44f, 	 0.0f, 0.0f,     -0.8f, 0.5f,  0.0f, // Left Side
                -0.5f, 0.0f, -0.5f,     0.83f, 0.70f, 0.44f,	 5.0f, 0.0f,     -0.8f, 0.5f,  0.0f, // Left Side
                 0.0f, 0.8f,  0.0f,     0.92f, 0.86f, 0.76f,	 2.5f, 5.0f,     -0.8f, 0.5f,  0.0f, // Left Side
            
                -0.5f, 0.0f, -0.5f,     0.83f, 0.70f, 0.44f,	 5.0f, 0.0f,      0.0f, 0.5f, -0.8f, // Non-facing side
                 0.5f, 0.0f, -0.5f,     0.83f, 0.70f, 0.44f,	 0.0f, 0.0f,      0.0f, 0.5f, -0.8f, // Non-facing side
                 0.0f, 0.8f,  0.0f,     0.92f, 0.86f, 0.76f,	 2.5f, 5.0f,      0.0f, 0.5f, -0.8f, // Non-facing side
            
                 0.5f, 0.0f, -0.5f,     0.83f, 0.70f, 0.44f,	 0.0f, 0.0f,      0.8f, 0.5f,  0.0f, // Right side
                 0.5f, 0.0f,  0.5f,     0.83f, 0.70f, 0.44f,	 5.0f, 0.0f,      0.8f, 0.5f,  0.0f, // Right side
                 0.0f, 0.8f,  0.0f,     0.92f, 0.86f, 0.76f,	 2.5f, 5.0f,      0.8f, 0.5f,  0.0f, // Right side
            
                 0.5f, 0.0f,  0.5f,     0.83f, 0.70f, 0.44f,	 5.0f, 0.0f,      0.0f, 0.5f,  0.8f, // Facing side
                -0.5f, 0.0f,  0.5f,     0.83f, 0.70f, 0.44f, 	 0.0f, 0.0f,      0.0f, 0.5f,  0.8f, // Facing side
                 0.0f, 0.8f,  0.0f,     0.92f, 0.86f, 0.76f,	 2.5f, 5.0f,      0.0f, 0.5f,  0.8f  // Facing side
            };
            
            // Indices for vertices order
            GLuint indices[] =
            {
                0, 1, 2, // Bottom side
                0, 2, 3, // Bottom side
                4, 6, 5, // Left side
                7, 9, 8, // Non-facing side
                10, 12, 11, // Right side
                13, 15, 14 // Facing side
            };

            amountOfVertices = sizeof(indices) / sizeof(GL_UNSIGNED_INT);

            VAO1 = VAO();

            VAO1.bind();

            VBO VBO1(vertices, sizeof(vertices));
            EBO EBO1(indices, sizeof(indices));

            VAO1.linkAttrib(VBO1, 0, 3, GL_FLOAT, 11 * sizeof(float), (void*)0);
            VAO1.linkAttrib(VBO1, 1, 3, GL_FLOAT, 11 * sizeof(float), (void*)(3 * sizeof(float)));
            VAO1.linkAttrib(VBO1, 2, 2, GL_FLOAT, 11 * sizeof(float), (void*)(6 * sizeof(float)));
            VAO1.linkAttrib(VBO1, 3, 3, GL_FLOAT, 11 * sizeof(float), (void*)(8 * sizeof(float)));

            VAO1.unbind();
            VBO1.unbind();
            EBO1.unbind();

        }

        void draw() {
            shader->activate();

            shader->applyModelMatrix(objectModelMatrix);

            VAO1.bind();

            glDrawElements(GL_TRIANGLES, amountOfVertices, GL_UNSIGNED_INT, 0);

            VAO1.unbind();
        }
    private:
        VAO VAO1;
        Shader* shader;
        int amountOfVertices;
};
