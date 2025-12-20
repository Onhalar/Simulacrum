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
#include <3DModelImport.hpp> // For ModelData and loadSTLData
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

public:
    ModelData modelData;      // Raw model data (vertices, normals, indices)
    glm::vec3 color;     // The base color of the object


    Model(ModelData data, const glm::vec3& color)
        : modelData(data), color(color), vao(nullptr), vboPositions(nullptr),
          vboNormals(nullptr), vboColors(nullptr), ebo(nullptr)
    {
        // Pointers are initialized to nullptr, the OpenGL objects are not created here.
    }

    Model(const Model& copy) : modelData(copy.modelData), color(copy.color), vao(nullptr), vboPositions(nullptr),
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
        
        // model data gets automatically deleted
        clearBufferedData();
        
    }

    void clearBufferedData() {
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
    
    void sendBufferedVertices() {
        if (modelData.indices.empty() && debugMode) {
            std::cerr << formatError("ERROR") << ": ModelData has no verticies, cannot buffer vertices." << std::endl;
            return;
        }
        if (vao && debugMode) {
            std::cout << formatWarning("WARNING") << ": Rewriting buffered vertices." << std::endl;
            clearBufferedData();
        }

        // Create VAO and bind it
        vao = new VAO();
        vao->bind();

        // --- Vertex Positions (location = 0) ---
        vboPositions = new VBO(modelData.vertices.data(), modelData.vertices.size() * sizeof(GLfloat));
        vao->linkAttrib(*vboPositions, 0, 3, GL_FLOAT, 3 * sizeof(GLfloat), (void*)0);

        // --- Vertex Colors (location = 1) ---
        std::vector<GLfloat> colors;
        colors.reserve(modelData.vertices.size()); // 3 components per vertex
        for (size_t i = 0; i < modelData.vertices.size() / 3; ++i) {
            colors.push_back(color.r); // R
            colors.push_back(color.g); // G
            colors.push_back(color.b); // B
        }
        vboColors = new VBO(colors.data(), colors.size() * sizeof(GLfloat));
        vao->linkAttrib(*vboColors, 1, 3, GL_FLOAT, 3 * sizeof(GLfloat), (void*)0);

        // --- Normals (location = 2) ---
        vboNormals = new VBO(modelData.normals.data(), modelData.normals.size() * sizeof(GLfloat));
        vao->linkAttrib(*vboNormals, 2, 3, GL_FLOAT, 3 * sizeof(GLfloat), (void*)0);

        // --- EBO (Indices) ---
        ebo = new EBO(modelData.indices.data(), modelData.indices.size() * sizeof(GLuint));

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

        if (!vao || !ebo || modelData.indices.empty()) {
            // Model not properly loaded or initialized, cannot draw.
            std::cerr << formatError("ERROR") << ": Attempted to draw a model that has not been buffered." << std::endl;
            return;
        }

        shader->activate(); // Activate the shader program

        vao->bind(); // Bind the VAO

        glDrawElements(GL_TRIANGLES, modelData.indices.size(), GL_UNSIGNED_INT, 0);

        // Unbind the VAO
        vao->unbind();
    }
};

#endif // MODEL_CLASS_HEADER
