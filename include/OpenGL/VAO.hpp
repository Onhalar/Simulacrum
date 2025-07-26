#ifndef VAO_CLASS_HEADER
#define VAO_CLASS_HEADER

#include <glad/glad.h>

#include "VBO.hpp"

class VAO {
    public:
        mutable GLuint ID;

        VAO() {
            glGenVertexArrays(1, &ID);
        }

        void linkAttrib(VBO& VBO, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset) {
            VBO.bind();
            // stride => number of bytes per vertex
            glVertexAttribPointer(layout, numComponents, type, GL_FALSE, stride, offset);
            glEnableVertexAttribArray(layout);
            VBO.unbind();
        }

        void bind() {
            glBindVertexArray(ID);
        }

        void unbind() {
            glBindVertexArray(0);
        }

        ~VAO() {
            glDeleteVertexArrays(1, &ID);
        }
};

#endif // VAO_CLASS_HEADER