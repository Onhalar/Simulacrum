#ifndef UBO_CLASS_HEADER
#define UBO_CLASS_HEADER

#include <glad/glad.h>

class UBO {
    public:
        mutable GLuint ID;

        UBO(GLsizeiptr size, const void* data = nullptr, GLenum usage = GL_DYNAMIC_DRAW) {
            glGenBuffers(1, &ID);
            glBindBuffer(GL_UNIFORM_BUFFER, ID);
            if (data) {
                glBufferData(GL_UNIFORM_BUFFER, size, data, usage);
            } else {
                glBufferData(GL_UNIFORM_BUFFER, size, nullptr, usage);
            }
        }

        void bind(GLuint bindingPoint = 0) const {
            glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, ID);
        }

        void unbind() const {
            glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0);  // Unbind from the binding point
        }

        void update(GLintptr offset, GLsizeiptr size, const void* data) {
            glBindBuffer(GL_UNIFORM_BUFFER, ID);
            glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
        }

        ~UBO() {
            glDeleteBuffers(1, &ID);
        }
};

#endif // UBO_CLASS_HEADER
