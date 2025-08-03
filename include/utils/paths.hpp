#ifndef PROJECT_PATH_HEADER
#define PROJECT_PATH_HEADER

#include <filesystem>
#include <string>

#include <config.hpp>

inline std::string getFileName(const std::string& path) {
    return std::filesystem::path(path).filename().string();
}

inline std::string getFileName(const std::filesystem::path path) {
    return std::filesystem::path(path).filename().string();
}

#endif // PROJECT_PATH_HEADER