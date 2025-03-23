cmake_minimum_required(VERSION 3.26.0)
project(simulacrum VERSION 1.0.0)

# Try to find OpenGL package
find_package(OpenGL QUIET)
find_package(glfw3 REQUIRED)

add_executable(
    simulacrum

    # main entry point of the application
    src/main.cpp

    #dependencies
    dependencies/glad.c
    )

target_include_directories(simulacrum PRIVATE include/core include/OpenGL include/utils dependencies )

# Check if OpenGL was found
if (OPENGL_FOUND)
    target_link_libraries(simulacrum OpenGL::GL)
else()
    message(STATUS "OpenGL not found, using opengl32.")
    target_link_libraries(simulacrum opengl32)
endif()

target_link_libraries(simulacrum glfw3)