#ifndef MODEL_CLASS_HEADER
#define MODEL_CLASS_HEADER

#include <string>
#include <vector>
#include <iostream> // For error output
#include <filesystem>

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
#include <debug.hpp>

/**
 * @brief A class to encapsulate the loading, storage, and rendering of a 3D model
 * loaded from an STL file.
 */
class Model {
private:
    VAO* vao;                  // Vertex Array Object
    VBO* vboPositions;         // VBO for vertex positions
    VBO* vboNormals;           // VBO for vertex normals
    VBO* vboColors;            // VBO for vertex colors
    EBO* ebo;                  // Element Buffer Object (indices)
    glm::vec3 objectColor;     // The base color of the object

public:
    ModelData* modelData;      // Raw model data (vertices, normals, indices)

    /**
     * @brief Constructor for the Model class. It now only stores the raw model
     * data and color, and initializes OpenGL buffer pointers to null.
     * The actual OpenGL buffer creation is deferred to the sendBufferedVertices() method.
     * @param data A pointer to the ModelData struct containing vertex, normal, and index data.
     * @param color The base color for the model.
     */
    Model(ModelData* data, const glm::vec3& color)
        : modelData(data), objectColor(color), vao(nullptr), vboPositions(nullptr),
          vboNormals(nullptr), vboColors(nullptr), ebo(nullptr)
    {
        // Pointers are initialized to nullptr, the OpenGL objects are not created here.
    }

    /**
     * @brief Destructor for the Model class. Cleans up all allocated resources.
     */
    ~Model() {
        // Only delete the resources if they were actually allocated.
        // This makes the destructor safe even if sendBufferedVertices() was never called.
        if (modelData) {
            delete modelData;
            modelData = nullptr;
        }
        if (vao) {
            delete vao;
            vao = nullptr;
        }
        if (vboPositions) {
            delete vboPositions;
            vboPositions = nullptr;
        }
        if (vboNormals) {
            delete vboNormals;
            vboNormals = nullptr;
        }
        if (vboColors) {
            delete vboColors;
            vboColors = nullptr;
        }
        if (ebo) {
            delete ebo;
            ebo = nullptr;
        }
    }

    /**
     * @brief Creates and populates the OpenGL buffers (VAO, VBOs, EBO) with the
     * model's data. This function must be called after the OpenGL context is created.
     * This method is idempotent, it will only buffer the data once.
     */
    void sendBufferedVertices() {
        if (!modelData && debugMode) {
            std::cerr << formatError("ERROR") << ": ModelData is null, cannot buffer vertices." << std::endl;
            return;
        }
        if (vao && debugMode) {
            std::cout << formatWarning("WARNING") << ": Vertices already buffered for this model. Skipping." << std::endl;
            return;
        }

        // Create VAO and bind it
        vao = new VAO();
        vao->bind();

        // --- Vertex Positions (location = 0) ---
        vboPositions = new VBO(modelData->vertices.data(), modelData->vertices.size() * sizeof(GLfloat));
        vao->linkAttrib(*vboPositions, 0, 3, GL_FLOAT, 3 * sizeof(GLfloat), (void*)0);

        // --- Vertex Colors (location = 1) ---
        std::vector<GLfloat> colors;
        colors.reserve(modelData->vertices.size()); // 3 components per vertex
        for (size_t i = 0; i < modelData->vertices.size() / 3; ++i) {
            colors.push_back(objectColor.r); // R
            colors.push_back(objectColor.g); // G
            colors.push_back(objectColor.b); // B
        }
        vboColors = new VBO(colors.data(), colors.size() * sizeof(GLfloat));
        vao->linkAttrib(*vboColors, 1, 3, GL_FLOAT, 3 * sizeof(GLfloat), (void*)0);

        // --- Normals (location = 2) ---
        vboNormals = new VBO(modelData->normals.data(), modelData->normals.size() * sizeof(GLfloat));
        vao->linkAttrib(*vboNormals, 2, 3, GL_FLOAT, 3 * sizeof(GLfloat), (void*)0);

        // --- EBO (Indices) ---
        ebo = new EBO(modelData->indices.data(), modelData->indices.size() * sizeof(GLuint));

        // Unbind everything
        vao->unbind();
        vboPositions->unbind();
        vboColors->unbind();
        vboNormals->unbind();
        ebo->unbind();
    }


    /**
     * @brief Draws the model.
     * @param shader The shader program to use for rendering.
     */
    void draw(Shader* shader) {

        if (!vao || !ebo || !modelData) {
            // Model not properly loaded or initialized, cannot draw.
            std::cerr << formatError("ERROR") << ": Attempted to draw a model that has not been buffered." << std::endl;
            return;
        }

        shader->activate(); // Activate the shader program

        vao->bind(); // Bind the VAO

        glDrawElements(GL_TRIANGLES, modelData->indices.size(), GL_UNSIGNED_INT, 0);

        // Unbind the VAO
        vao->unbind();
    }
};

#endif // MODEL_CLASS_HEADER
