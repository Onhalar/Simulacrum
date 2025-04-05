cmake_minimum_required(VERSION 3.26.0)
project(simulacrum VERSION 1.0.0)

# Try to find OpenGL package
find_package(OpenGL QUIET)

add_executable(
    simulacrum

    # main entry point of the application
    src/main.cpp

    # dependencies
    dependencies/glad.c
)

target_include_directories(simulacrum PRIVATE
    include/core
    include/OpenGL
    include/utils
    include/objects

    dependencies
    )

# Set the output directory for the executable to "bin" inside the build folder
set_target_properties(simulacrum PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Check if OpenGL was found
if (OPENGL_FOUND)
    target_link_libraries(simulacrum OpenGL::GL)
else()
    message(STATUS "OpenGL not found, using opengl32.")
    target_link_libraries(simulacrum opengl32)
endif()


if(WIN32)
    target_link_libraries(simulacrum "${CMAKE_SOURCE_DIR}/dependencies/GLFW/libglfw3.a")
else()
    target_link_libraries(simulacrum "${CMAKE_SOURCE_DIR}/dependencies/GLFW/libglfw3-compiled-mint.a")
endif()

# did not have the time to test this yet so here goes
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")

# Define paths to the directories
set(SOURCE_RES_DIR "${CMAKE_SOURCE_DIR}/res")
set(SOURCE_SHADERS_DIR "${CMAKE_SOURCE_DIR}/shaders")
set(BUILD_RES_DIR "${CMAKE_BINARY_DIR}/res")
set(BUILD_SHADERS_DIR "${CMAKE_BINARY_DIR}/shaders")

# Create a custom command to copy the res directory
add_custom_command(
    TARGET simulacrum PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${SOURCE_RES_DIR} ${BUILD_RES_DIR}
)

# Create a custom command to copy the shaders directory
add_custom_command(
    TARGET simulacrum PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${SOURCE_SHADERS_DIR} ${BUILD_SHADERS_DIR}
)

# Install the main executable
install(TARGETS simulacrum DESTINATION bin)
