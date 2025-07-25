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
    float lightDistance;        // Falloff factor (can be used to control the effect of distance)
};

void main() {
    vec3 totalAmbient = vec3(0.0);
    vec3 totalDiffuse = vec3(0.0);

    // --- Ambient Lighting (Global or Per-Light) ---
    // You can have a global ambient light or calculate ambient per light.
    // For simplicity, we'll calculate per-light ambient here.
    float ambientStrength = 0.1; // How strong the ambient light is

    vec3 norm = normalize(normal); // Normalize the normal vector (important!)

    // Loop through all active lights
    for (int i = 0; i < lightCount; ++i) {
        vec3 currentLightColor = lights[i].color.rgb;
        vec3 currentLightPosition = lights[i].position;
        float currentLightIntensity = lights[i].intensity;

        // Calculate ambient contribution for this light
        totalAmbient += ambientStrength * currentLightColor * currentLightIntensity;

        // --- Diffuse Lighting ---
        vec3 lightDir = normalize(currentLightPosition - currentPosition); // Direction from fragment to light
        float diff = max(dot(norm, lightDir), 0.0); // Calculate dot product, clamp to 0 if negative
        totalDiffuse += diff * currentLightColor * currentLightIntensity;
    }

    // Combine all lighting components
    vec3 result = (totalAmbient + totalDiffuse) * color; // Multiply by the object's base color

    FragColor = vec4(result, 1.0); // Output the final lit color
}
