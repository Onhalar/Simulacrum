#ifndef DEBUG_HEADER
#define DEBUG_HEADER

#ifdef DEBUG_ENABLED
    bool debugMode = true;
#else
    bool debugMode = false;
#endif

bool prettyOutput = true;

#include <glm/glm.hpp>
#include <iostream>

inline void printVec3(const char* name, glm::vec3 in) {
    std::cout << name << ": " << in.x << ", " << in.y << ", " << in.z << std::endl;
}
inline void printVec3(const char* name, glm::dvec3 in) {
    std::cout << name << ": " << in.x << ", " << in.y << ", " << in.z << std::endl;
}

template <typename T>
inline void print(T output) {
    std::cout << output << std::endl;
}

#endif // DEBUG_HEADER