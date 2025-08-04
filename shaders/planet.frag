#version 330 core

in vec3 color;            // The vertex color (can be used as object's base color)
in vec3 normal;           // The interpolated normal vector at this fragment (world space)
in vec3 currentPosition;  // The interpolated position of the current fragment (world space)

out vec4 FragColor;       // The output color of the fragment

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
    float lightFallOff;       // Falloff factor (can be used to control the effect of distance)
};

void main() {
    vec3 totalAmbient = vec3(0.0);
    vec3 totalDiffuse = vec3(0.0);

    // Calculate simple ambient occlusion factor
    vec3 norm = normalize(normal);

    // Base ambient strength (will be modified by AO)
    float ambientStrength = 0.1;

    // Loop through all active lights
    for (int i = 0; i < lightCount; ++i) {
        vec3 currentLightColor = lights[i].color.rgb;
        vec3 currentLightPosition = lights[i].position;
        float currentLightIntensity = lights[i].intensity;

        // Calculate distance to light and falloff
        float distance = length(currentLightPosition - currentPosition);
        
        // Enhanced attenuation calculation using lightFallOff uniform
        float attenuation = 1.0 / (1.0 + 0.1 * lightFallOff * 0.01 * distance * distance);
        
        // Apply light intensity to attenuation
        attenuation *= currentLightIntensity;

        // --- Diffuse Lighting ---
        vec3 lightDir = normalize(currentLightPosition - currentPosition);
        float diff = max(dot(norm, lightDir), 0.0);
        totalDiffuse += diff * currentLightColor * attenuation;
    }

    // Combine all lighting components
    vec3 result = max(totalDiffuse, ambientStrength) * color;

    FragColor = vec4(result, 1.0);
}