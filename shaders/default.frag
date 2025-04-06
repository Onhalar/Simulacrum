#version 330 core

in vec2 texturePosition;

in vec3 normal;
in vec3 currentPosition;

out vec4 FragColor;

uniform sampler2D texture0;

uniform vec4 lightColor;
uniform vec3 lightPosition;
uniform float lightIntensity;
uniform vec3 cameraPosition;

void main() {

    // diffused lighting ---------------------------------------------------------

    // we're in space so no real need for it but let's just keep it there just in case
    float ambientLight = 0.20f;

    vec3 faceNormal = normalize(normal);
    vec3 lightDirection = normalize(lightPosition - currentPosition);

    float diffuse = max(dot(faceNormal, lightDirection), 0.0f) * lightIntensity;

    // specular lighting ---------------------------------------------------------

    float specularLight = 0.50f; // max amount of specular light

    vec3 viewDirection = normalize(cameraPosition - currentPosition);
    vec3 reflectionDirection = reflect(-lightDirection, normal);

    float specularAmount = pow(max(dot(viewDirection, reflectionDirection), 0.0f), 8);

    float specular = specularAmount * specularLight * lightIntensity;

    // combine all lighting effects ----------------------------------------------

    FragColor = texture(texture0, texturePosition) * lightColor * (diffuse + ambientLight + specular);
}