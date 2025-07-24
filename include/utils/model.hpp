#ifndef IMPORT_MODEL_HPP
#define IMPORT_MODEL_HPP

#include <string>
#include <vector>
#include <iostream> // For error output

// OpenGL and GLM includes
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Your existing OpenGL wrapper classes
#include <VAO.hpp>
#include <VBO.hpp>
#include <EBO.hpp>
#include <shader.hpp> // For the Shader class
#include <stlImport.hpp> // For ModelData and loadSTLData

/**
 * @brief A class to encapsulate the loading, storage, and rendering of a 3D model
 * loaded from an STL file.
 */
class Model {
private:
    ModelData* modelData;      // Raw model data (vertices, normals, indices)
    VAO* vao;                  // Vertex Array Object
    VBO* vboPositions;         // VBO for vertex positions
    VBO* vboNormals;           // VBO for vertex normals
    VBO* vboColors;            // VBO for vertex colors
    EBO* ebo;                  // Element Buffer Object (indices)
    glm::vec3 objectColor;     // The base color of the object

public:
    /**
     * @brief Constructor for the Model class. Loads the STL file and sets up OpenGL buffers.
     * @param filePath The path to the STL model file.
     * @param defaultColor The default color for the model (defaults to red).
     */
    Model(const std::string& filePath, const glm::vec3& defaultColor = glm::vec3(1.0f, 0.0f, 0.0f))
        : modelData(nullptr), vao(nullptr), vboPositions(nullptr), vboNormals(nullptr),
          vboColors(nullptr), ebo(nullptr), objectColor(defaultColor) {

        // Load the STL model data
        modelData = loadSTLData(filePath);

        if (!modelData) {
            std::cerr << "ERROR: Failed to load model from " << filePath << ". Model will not be renderable." << std::endl;
            return; // Exit constructor if loading failed
        }

        // Initialize OpenGL objects
        vao = new VAO();
        vao->bind(); // Bind the VAO first to configure its attributes

        // Create VBO for vertex positions and link to layout 0
        vboPositions = new VBO(modelData->vertices.data(), modelData->vertices.size() * sizeof(GLfloat));
        vao->linkAttrib(*vboPositions, 0, 3, GL_FLOAT, 3 * sizeof(GLfloat), (void*)0);

        // Generate color data for the vertices (using the provided defaultColor)
        std::vector<GLfloat> colors;
        colors.reserve(modelData->vertices.size()); // 3 color components per vertex
        for (size_t i = 0; i < modelData->vertices.size() / 3; ++i) {
            colors.push_back(objectColor.r);
            colors.push_back(objectColor.g);
            colors.push_back(objectColor.b);
        }

        // Create VBO for vertex colors and link to layout 1
        vboColors = new VBO(colors.data(), colors.size() * sizeof(GLfloat));
        vao->linkAttrib(*vboColors, 1, 3, GL_FLOAT, 3 * sizeof(GLfloat), (void*)0);

        // Create VBO for vertex normals and link to layout 2
        vboNormals = new VBO(modelData->normals.data(), modelData->normals.size() * sizeof(GLfloat));
        vao->linkAttrib(*vboNormals, 2, 3, GL_FLOAT, 3 * sizeof(GLfloat), (void*)0);

        // Create EBO for indices
        ebo = new EBO(modelData->indices.data(), modelData->indices.size() * sizeof(GLuint));

        // Unbind the VAO, VBOs, and EBO to prevent accidental modification
        vao->unbind();
        vboPositions->unbind();
        vboNormals->unbind();
        vboColors->unbind();
        ebo->unbind(); // IMPORTANT: Ensure EBO::unbind() correctly binds to 0
    }

    /**
     * @brief Destructor for the Model class. Cleans up all allocated resources.
     */
    ~Model() {
        if (modelData) {
            delete modelData;
            modelData = nullptr;
        }
        if (vao) {
            vao->destory(); // Note: Your VAO class has 'destory' typo.
            delete vao;
            vao = nullptr;
        }
        if (vboPositions) {
            vboPositions->destroy();
            delete vboPositions;
            vboPositions = nullptr;
        }
        if (vboNormals) {
            vboNormals->destroy();
            delete vboNormals;
            vboNormals = nullptr;
        }
        if (vboColors) {
            vboColors->destroy();
            delete vboColors;
            vboColors = nullptr;
        }
        if (ebo) {
            ebo->destroy();
            delete ebo;
            ebo = nullptr;
        }
    }

    /**
     * @brief Draws the model.
     * @param shader The shader program to use for rendering.
     * @param modelMatrix The model matrix for positioning and orienting the model.
     * @param lightColor The color of the light source.
     * @param lightPosition The position of the light source.
     * @param cameraPosition The position of the camera.
     */
    void draw(Shader* shader, const glm::mat4& modelMatrix,
              const glm::vec3& lightColor, const glm::vec3& lightPosition,
              const glm::vec3& cameraPosition) {

        if (!vao || !ebo || !modelData) {
            // Model not properly loaded or initialized, cannot draw.
            return;
        }

        shader->activate(); // Activate the shader program

        // Set lighting uniforms
        shader->setUniform("lightColor", lightColor);
        shader->setUniform("lightPosition", lightPosition);
        shader->setUniform("cameraPosition", cameraPosition);

        // Set the object's base color
        shader->setUniform("objectColor", objectColor);

        // Apply the model matrix
        shader->applyModelMatrix(modelMatrix);

        vao->bind(); // Bind the VAO
        glDrawElements(GL_TRIANGLES, modelData->indices.size(), GL_UNSIGNED_INT, 0);
        vao->unbind(); // Unbind the VAO
    }
};

#endif // IMPORT_MODEL_HPP
