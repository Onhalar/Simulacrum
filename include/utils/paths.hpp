#include <config.hpp>

string getFileName(const string& path) {
    return filesystem::path(path).filename().string();
}

string projectPath(const string& path) {
    return (projectDir / filesystem::path(path)).string();
}