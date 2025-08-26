#ifndef GLOBAL_SIMPLE_TYPE_HEADER
#define GLOBAL_SIMPLE_TYPE_HEADER

#include <string>
#include <tuple>
#include <vector>

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <color.hpp>

using ShaderID = std::string;
using ModelID = std::string;
using SimObjectID = std::string;
using SceneID = std::string;

class units {
    public:
        using kilometers = double;
        using tons = double;

        using meters = double;
        using kilograms = double;
};

using TinyInt = unsigned char; // 0-255

// --- STRUCTS ---
// structs may be later added into their own separate files

#include <glad/glad.h>

struct ModelData {
    std::vector<float> vertices; // Stores vertex positions (x, y, z)
    std::vector<float> normals;  // Stores vertex normals (nx, ny, nz)
    std::vector<unsigned int> indices;   // Stores indices for indexed drawing
};

// --- ENUMS ---

enum simulationType {
    realistic,
    simplified
};

#endif // GLOBAL_SIMPLE_TYPE_HEADER