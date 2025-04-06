#version 330 core

in vec2 texturePosition;  // The texture coordinates for the current fragment
in vec3 normal;           // The normal vector at this fragment
in vec3 currentPosition;  // The position of the current fragment (world space)

out vec4 FragColor;  // The output color of the fragment

uniform sampler2D texture0;  // The texture to sample
uniform vec3 cameraPosition;  // The position of the camera (view position)

#define MAX_LIGHTS 4  // Maximum number of lights we will process

// Light structure definition
struct Light {
    vec3 position;   // Position of the light in world space
    vec4 color;      // Color of the light (includes RGB)
    float intensity; // Intensity of the light
};

layout(std140) uniform LightBlock {
    Light lights[MAX_LIGHTS];  // Array of lights in UBO
    int lightCount;            // The actual number of active lights
};

void main() {
    // Normalize the normal vector for lighting calculations
    vec3 faceNormal = normalize(normal);
    vec3 viewDirection = normalize(cameraPosition - currentPosition);

    // Ambient lighting contribution (low, constant light to simulate background light)
    float ambientLight = 0.1f;  // You can increase or decrease this

    // Sample the texture color at the given texture coordinates
    vec4 texColor = texture(texture0, texturePosition);

    // Variables to accumulate the total diffuse and specular lighting
    vec3 totalDiffuse = vec3(0.0f);
    vec3 totalSpecular = vec3(0.0f);

    // Loop over the number of active lights and calculate lighting for each
    for (int i = 0; i < lightCount; ++i) {
        // Get the direction from the current fragment to the light source
        vec3 lightDirection = normalize(lights[i].position - currentPosition);

        // Calculate diffuse lighting (Lambertian reflectance)
        float diffuse = max(dot(faceNormal, lightDirection), 0.0f) * lights[i].intensity;

        // Calculate specular lighting (using Phong model)
        vec3 reflectionDirection = reflect(-lightDirection, faceNormal);
        float specularAmount = pow(max(dot(viewDirection, reflectionDirection), 0.0f), 8.0);
        float specular = specularAmount * 0.5f * lights[i].intensity;

        // Accumulate the lighting contributions, scaling by light color
        totalDiffuse += diffuse * lights[i].color.rgb;
        totalSpecular += specular * lights[i].color.rgb;
    }

    // Combine all lighting effects: ambient, diffuse, and specular
    vec3 totalLighting = (ambientLight + totalDiffuse + totalSpecular);

    // Final result color: combine texture color with the lighting
    vec4 resultColor = texColor * vec4(totalLighting, 1.0f);

    // Final output color of the fragment
    FragColor = resultColor;
}
