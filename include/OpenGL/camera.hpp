# ifndef CAMERA_CLASS_HEADER
# define CAMERA_CLASS_HEADER

// for delta time
#include <globals.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>


#include <shader.hpp>

class Camera {
    public:
        glm::mat4 projectionMatrix = glm::mat4(1.0f);
        glm::mat4 viewMatrix = glm::mat4(1.0f);

        glm::vec3 position;
        glm::vec3 orientation = glm::vec3(0.0f, 0.0f, -1.0f);
        const glm::vec3 UP = glm::vec3(0.0f, 1.0f, 0.0f);

        float cameraSpeed = 1.0f;
        float sensitivity = 175.0f;

        float nearClipPlane = 0.1f;
        float farClipPlane = 100.0f;

        int width, height;

        float FOVdeg = 45.0f;

        Camera(int width, int height, glm::vec3 position) {
            this->position = position;
        }

        void updateProjection(Shader* shader) {
            shader->activate();

            viewMatrix = glm::lookAt(position, position + orientation, UP);
            projectionMatrix = glm::perspective(glm::radians(FOVdeg), width/(float)height, nearClipPlane, farClipPlane);

            shader->viewMatrix = viewMatrix;
            shader->projectionMatrix = projectionMatrix;

            shader->applyViewMatrix();
            shader->applyProjectionMatrix();
        }
        void updateProjection(int projectionWidth, int projectionHeight, Shader* shader) {
            shader->activate();
            
            width = projectionWidth;
            height = projectionHeight;

            glViewport(0, 0, projectionWidth, projectionHeight);

            viewMatrix = glm::lookAt(position, position + orientation, UP);
            projectionMatrix = glm::perspective(glm::radians(FOVdeg), width/(float)height, nearClipPlane, farClipPlane);

            shader->viewMatrix = viewMatrix;
            shader->projectionMatrix = projectionMatrix;

            shader->applyViewMatrix();
            shader->applyProjectionMatrix();
        }
        void handleInputs(GLFWwindow* window, Shader* shader) {
            shader->activate();
            static bool controlCamera = false;

            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) && !controlCamera) {
                controlCamera = true;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                glfwSetCursorPos(window, (width / 2), (height / 2));
            }
            else if (glfwGetKey(window, GLFW_KEY_ESCAPE) && controlCamera) {
                controlCamera = false;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }

            if (controlCamera) {
                double mouseX, mouseY;
                glfwGetCursorPos(window, &mouseX, &mouseY);

                float rotX = sensitivity * (float)(mouseY - (height / 2)) / height;
                float rotY = sensitivity * (float)(mouseX - (width / 2)) / width;

                glm::vec3 newOrientation = glm::rotate(orientation, glm::radians(-rotX), glm::normalize(glm::cross(orientation, UP)));

                // Decides whether or not the next vertical Orientation is legal or not
                if (abs(glm::angle(newOrientation, UP) - glm::radians(90.0f)) <= glm::radians(85.0f))
                {
                    orientation = newOrientation;
                }

                glfwSetCursorPos(window, (width / 2), (height / 2));

                float FdeltaTime = (float)deltaTime;

                // Rotates the Orientation left and right
                orientation = glm::rotate(orientation, glm::radians(-rotY), UP);

                // forward [W]
                if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
                    position += orientation * cameraSpeed * FdeltaTime;
                }
                // backward [S]
                if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
                    position += orientation * -cameraSpeed * FdeltaTime;
                }

                // left [A]
                if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
                    position += -glm::normalize(glm::cross(orientation, UP)) * cameraSpeed * FdeltaTime;
                }
                // right [D]
                if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
                    position += glm::normalize(glm::cross(orientation, UP)) * cameraSpeed * FdeltaTime;
                }

                // up [SPACE]
                if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
                    position += cameraSpeed * UP * FdeltaTime;
                }
                // down [C]
                if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
                    position += cameraSpeed * -UP * FdeltaTime;
                }

                updateProjection(shader);
            }
        }
};

#endif
