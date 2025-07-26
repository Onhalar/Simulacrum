#ifndef EBO_CLASS_HEADER
#define EBO_CLASS_HEADER

#include <glad/glad.h>

class EBO {
    public:
        mutable GLuint ID;

        EBO(GLuint* indicies, GLsizeiptr size) {
            glGenBuffers(1, &ID);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indicies, GL_STATIC_DRAW);
        }

        void bind() {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
        }

        void unbind() {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
        }

        ~EBO() {
            glDeleteBuffers(1, &ID);
        }
};

#endif // EBO_CLASS_HEADER