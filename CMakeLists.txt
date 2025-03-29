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

target_include_directories(simulacrum PRIVATE include/core include/OpenGL include/utils dependencies)

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


# Install the main executable
install(TARGETS simulacrum DESTINATION bin)

# Copy /res and /shaders folders to the install directory
file(GLOB_RECURSE RESOURCES "${CMAKE_SOURCE_DIR}/res/*")
file(GLOB_RECURSE SHADERS "${CMAKE_SOURCE_DIR}/shaders/*")

foreach(FILE ${RESOURCES})
    get_filename_component(FILENAME ${FILE} NAME)
    install(FILES ${FILE} DESTINATION res)
endforeach()

foreach(FILE ${SHADERS})
    get_filename_component(FILENAME ${FILE} NAME)
    install(FILES ${FILE} DESTINATION shaders)
endforeach()

