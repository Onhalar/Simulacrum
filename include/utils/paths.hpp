#include <filesystem>
#include <string>

std::string getFileName(const std::string& path) {
    return filesystem::path(path).filename().string();
}

std::string projectPath(const std::string& path) {
    return (projectDir / filesystem::path(path)).string();
}