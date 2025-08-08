#version 330 core

in vec3 color;
in vec3 normal;
in vec3 currentPosition;

out vec4 FragColor;

void main() {
    FragColor = vec4(color, 1);
}