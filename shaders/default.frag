#version 330 core

in vec3 color;
in vec2 texturePosition;

in vec3 normal;
in vec3 currentPosition;

out vec4 FragColor;

uniform sampler2D texture0;
uniform vec4 lightColor;
uniform vec3 lightPosition;
uniform float lightIntensity;

void main() {
    vec3 faceNormal = normalize(normal);
    vec3 lightDirection = normalize(lightPosition - currentPosition);

    float diffuse = max(dot(faceNormal, lightDirection), 0.0f) * lightIntensity;

    FragColor = texture(texture0, texturePosition) * lightColor * diffuse;
}