#ifndef LOADED_RESOURCES_HEADER
#define LOADED_RESOURCES_HEADER

#include <simObject.hpp>

#include <unordered_map>

using SimObjectList = std::unordered_map<std::string, simulationObject*>;

SimObjectList SimObjects;

#endif // LOADED_RESOURCES_HEADER