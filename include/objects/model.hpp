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

    void handleFlags(const unsigned int flags) {
        if (flags & Model::Flags::MAKE_INSTANCE) {
            isDerived = true;
            transform = glm::mat4(1.0f);
        }
    }

    Model* findOriginalMaster(Model* master) {
        while (true) {
            if (master && master->isDerived) {
                master = master->master;
            }
            else {
                break;
            }
        }
        return master;
    }

public:
    struct Flags {
        static const unsigned int MAKE_INSTANCE = 0b00000001;
    };

    ModelData modelData;      // Raw model data (vertices, normals, indices)
    glm::vec3 color;     // The base color of the object

    // for instances
    Model* master = nullptr;
    bool isDerived = false;
    glm::mat4 transform;


    Model(ModelData data, const glm::vec3& color, const unsigned int flags = 0)
        : modelData(data), color(color), vao(nullptr), vboPositions(nullptr),
          vboNormals(nullptr), vboColors(nullptr), ebo(nullptr)
    {
        // Pointers are initialized to nullptr, the OpenGL objects are not created here.
        handleFlags(flags);
    }

    Model(Model& master, const unsigned int flags = 0) : color(master.color), vao(nullptr), vboPositions(nullptr),
          vboNormals(nullptr), vboColors(nullptr), ebo(nullptr)
    {
        // Pointers are initialized to nullptr, the OpenGL objects are not created here.
        handleFlags(flags);

        if (!isDerived) { this->modelData = master.modelData; }
        else { this->master = findOriginalMaster(&master); }
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
    
    void ensureMasterIsBuffered() {
        if (isDerived && master) {
            if (master->ebo && master->vao) { return; }

            master->sendBufferedVertices();
        }
    }

    void sendBufferedVertices() {
        if (isDerived) {
            ensureMasterIsBuffered();
            return;
        }

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

        // --- Normals (location = 1) ---
        vboNormals = new VBO(modelData.normals.data(), modelData.normals.size() * sizeof(GLfloat));
        vao->linkAttrib(*vboNormals, 1, 3, GL_FLOAT, 3 * sizeof(GLfloat), (void*)0);

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
    void draw(Shader* shader, bool skipDerivedMatrix = false, bool skipColor = false) {

        if ((!vao || !ebo) && !isDerived) {
            // Model not properly loaded or initialized, cannot draw.
            std::cerr << formatError("ERROR") << ": Attempted to draw a model that has not been buffered." << std::endl;
            return;
        }

        shader->activate(); // Activate the shader program
        if (!skipColor) { shader->setUniform("color", color); }

        if (!isDerived) {
            vao->bind(); // Bind the VAO

            glDrawElements(GL_TRIANGLES, modelData.indices.size(), GL_UNSIGNED_INT, 0);

            // Unbind the VAO
            vao->unbind();
        }
        else {
            if (!skipDerivedMatrix) { shader->applyModelMatrix(transform); }
            master->draw(shader, true, true);
        }
    }
};

#endif // MODEL_CLASS_HEADER
