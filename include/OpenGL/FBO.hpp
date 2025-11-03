#ifndef FBO_CLASS_HEADER
#define FBO_CLASS_HEADER

#include <glad/glad.h>
#include <vector>
#include <VAO.hpp>
#include <VBO.hpp>
#include <EBO.hpp>
#include <shader.hpp>
#include <camera.hpp>

class FBO {
    public:
        GLuint ID;
        GLuint textureID;
        GLuint rboID;

        VAO* quadVAO;
        VBO* quadVBO;
        EBO* quadEBO;

        FBO(GLsizei width, GLsizei height) {
            // Generate and bind framebuffer
            glGenFramebuffers(1, &ID);
            glBindFramebuffer(GL_FRAMEBUFFER, ID);
            
            // Create texture attachment
            glGenTextures(1, &textureID);
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

            // prevent texture tiling
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0);
            
            // Create renderbuffer object for depth and stencil attachment
            glGenRenderbuffers(1, &rboID);
            glBindRenderbuffer(GL_RENDERBUFFER, rboID);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboID);
            
            // Check if framebuffer is complete
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
                // Handle error - framebuffer is not complete
            }
            
            // Unbind framebuffer
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // Setup screen quad for drawing with EBO
            setupScreenQuad();
        }

        void setupScreenQuad() {
            // Vertex data for a fullscreen quad (positions and texture coordinates)
            float quadVertices[] = {
                // positions   // texCoords
                -1.0f,  1.0f,  0.0f, 1.0f, // top-left
                -1.0f, -1.0f,  0.0f, 0.0f, // bottom-left
                1.0f, -1.0f,  1.0f, 0.0f, // bottom-right
                1.0f,  1.0f,  1.0f, 1.0f  // top-right
            };

            // Indices for the quad (2 triangles)
            GLuint quadIndices[] = {
                0, 1, 2, // first triangle
                0, 2, 3  // second triangle
            };

            // Create VBO, EBO, and VAO
            quadVBO = new VBO(quadVertices, sizeof(quadVertices));
            quadEBO = new EBO(quadIndices, sizeof(quadIndices));
            quadVAO = new VAO();
            
            // Bind VAO first
            quadVAO->bind();
            
            // Link vertex attributes
            quadVBO->bind();
            
            // Position attribute (location = 0)
            quadVAO->linkAttrib(*quadVBO, 0, 2, GL_FLOAT, 4 * sizeof(float), (void*)0);
            
            // Texture coordinate attribute (location = 1)
            quadVAO->linkAttrib(*quadVBO, 1, 2, GL_FLOAT, 4 * sizeof(float), (void*)(2 * sizeof(float)));
            
            // Bind EBO while VAO is still bound
            quadEBO->bind();
            
            // Unbind everything
            quadVAO->unbind();
            quadVBO->unbind();
            quadEBO->unbind();
        }

        void bind() {
            glBindFramebuffer(GL_FRAMEBUFFER, ID);
        }

        void unbind() {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        GLuint getTexture() const {
            return textureID;
        }

        // always update the camera along with this
        void resize(const GLsizei& width, const GLsizei& height) {
            // Resize texture
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
            
            // Resize renderbuffer
            glBindRenderbuffer(GL_RENDERBUFFER, rboID);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
            
            glBindTexture(GL_TEXTURE_2D, 0);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
        }

        void draw(Shader* shader) {
            // Use the provided shader
            shader->activate();
            
            // Bind the FBO's texture
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureID);
            
            // Set the texture uniform (assuming it's named "screenTexture")
            shader->setUniform("screenTexture", 0);
            
            // Draw the screen quad using EBO
            quadVAO->bind();
            quadEBO->bind();
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            quadEBO->unbind();
            quadVAO->unbind();
        }

        void draw(Shader* shader, const char* textureUniformName) {
            // Use the provided shader
            shader->activate();
            
            // Bind the FBO's texture
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureID);
            
            // Set the texture uniform with custom name
            shader->setUniform(textureUniformName, 0);
            
            // Draw the screen quad using EBO
            quadVAO->bind();
            quadEBO->bind();
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            quadEBO->unbind();
            quadVAO->unbind();
        }

        ~FBO() {

            delete quadVAO;
            delete quadEBO;
            delete quadVBO;

            quadVAO = nullptr;
            quadEBO = nullptr;
            quadVBO = nullptr;

            glDeleteFramebuffers(1, &ID);
            glDeleteTextures(1, &textureID);
            glDeleteRenderbuffers(1, &rboID);
        }
};

#endif // FBO_CLASS_HEADER