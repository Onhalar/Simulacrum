#include <config.hpp>
#include <globals.hpp>

#include <VBO.hpp>
#include <EBO.hpp>
#include <VAO.hpp>

class dummyObject {
    private:
        GLuint ID;
    public:
        dummyObject() {
            int imageWidth, imageHeight, channelAmount;
            stbi_set_flip_vertically_on_load(true);
            unsigned char* pixels = stbi_load(projectPath("res/img/example.png").c_str(), &imageWidth, &imageHeight, &channelAmount, 0);
        
            glGenTextures(1, &ID);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, ID);
        
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageWidth, imageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
            glGenerateMipmap(GL_TEXTURE_2D);

            stbi_image_free(pixels);

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

            mainShader->activate();

            GLuint uniID = glGetUniformLocation(mainShader->ID, "scale");

            glUniform1f(uniID, 0.5f);

            GLuint texture0Uniform = glGetUniformLocation(mainShader->ID, "texture0");

            glUniform1i(texture0Uniform, 0);

        }

        void draw() {
            VAO1.bind();
            glBindTexture(GL_TEXTURE_2D, ID);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
    private:
        VAO VAO1;
};
