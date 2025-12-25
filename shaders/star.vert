#version 330 core

layout (location = 0) in vec3 vertexPos;
layout (location = 1) in vec3 faceNormal;

out vec3 normal;
out vec3 currentPosition;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    // Transform vertex position to world space
    currentPosition = vec3(model * vec4(vertexPos, 1.0f));

    normal = transpose(inverse(mat3(model))) * faceNormal;

    // Transform vertex to clip space
    gl_Position = projection * view * model * vec4(vertexPos, 1.0f);
}
