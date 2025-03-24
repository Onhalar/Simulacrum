#version 450 core

layout (location = 0) in vec3 vertexPos;
layout (location = 1) in vec3 vertexColor;
layout (location = 2) in vec2 vertexTexture;

out vec3 color;
out vec2 texturePosition;

uniform float scale;

void main() {
    gl_Position = vec4(
        vertexPos.x + vertexPos.x * scale,
        vertexPos.y + vertexPos.y * scale,
        vertexPos.z + vertexPos.z * scale,
        1.0
    );
    color = vertexColor;
    texturePosition = vertexTexture;
}