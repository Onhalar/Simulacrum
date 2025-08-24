#ifndef TEXTURE_CLASS_HEADER
#define TEXTURE_CLASS_HEADER

#include <glad/glad.h>

#include <shader.hpp>

class Texture {
    public:
        GLuint ID;
        GLenum type;

        Texture(const char* image, GLenum textureType, GLenum slot, GLenum format, GLenum pixelType) {
            // Assigns the type of the texture ot the texture object
            type = textureType;

            // Stores the width, height, and the number of color channels of the image
            int widthImg, heightImg, numColCh;
            // Flips the image so it appears right side up
            stbi_set_flip_vertically_on_load(true);
            // Reads the image from a file and stores it in bytes
            unsigned char* bytes = stbi_load(image, &widthImg, &heightImg, &numColCh, 0);

            // Generates an OpenGL texture object
            glGenTextures(1, &ID);
            // Assigns the texture to a Texture Unit
            glActiveTexture(slot);
            glBindTexture(textureType, ID);

            // Configures the type of algorithm that is used to make the image smaller or bigger
            glTexParameteri(textureType, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
            glTexParameteri(textureType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            // Configures the way the texture repeats (if it does at all)
            glTexParameteri(textureType, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(textureType, GL_TEXTURE_WRAP_T, GL_REPEAT);

            // Extra lines in case you choose to use GL_CLAMP_TO_BORDER
            // float flatColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
            // glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, flatColor);

            // Assigns the image to the OpenGL Texture object
            glTexImage2D(textureType, 0, GL_RGBA, widthImg, heightImg, 0, format, pixelType, bytes);
            // Generates MipMaps
            glGenerateMipmap(textureType);

            // Deletes the image data as it is already in the OpenGL Texture object
            stbi_image_free(bytes);

            // Unbinds the OpenGL Texture object so that it can't accidentally be modified
            glBindTexture(textureType, 0);
        }

        void textureUnit(Shader* shader, const char* uniform, GLuint uint) {
            // Gets the location of the uniform
            GLuint texUni = glGetUniformLocation(shader->ID, uniform);
            // Shader needs to be activated before changing the value of a uniform
            shader->activate();
            // Sets the value of the uniform
            glUniform1i(texUni, uint);
        }

        void bind() {
            glBindTexture(type, ID);
        }

        void unbind() {
            glBindTexture(type, 0);
        }

        ~Texture() {
            glDeleteTextures(1, &ID);
        }
};

#endif // TEXTURE_CLASS_HEADER