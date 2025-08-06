#version 330 core

in vec3 color;
in vec3 normal;
in vec3 currentPosition;

out vec4 FragColor;

#define MAX_LIGHTS 4

struct Light {
    vec3 position;
    vec4 color;
    float intensity;
};

layout(std140) uniform LightBlock {
    Light lights[MAX_LIGHTS];
    int lightCount;
    float lightFallOff;
};

// Additive blending function
vec3 blendAdditive(vec3 base, vec3 blend) {
    return min(base + blend, 1.0);
}

void main() {
    vec3 combinedLightColor = vec3(0.0);
    float totalDiffuse = 0.0;

    vec3 norm = normalize(normal);
    float ambientStrength = 0.2;

    // Loop through all active lights
    for (int i = 0; i < lightCount; ++i) {
        vec3 currentLightColor = lights[i].color.rgb;
        float distance = length(lights[i].position - currentPosition);
        float attenuation = 1.0 / (1.0 + 0.1 * lightFallOff * 0.01 * distance * distance);
        attenuation *= lights[i].intensity;

        vec3 lightDir = normalize(lights[i].position - currentPosition);
        float diff = max(dot(norm, lightDir), 0.0);
        
        // Accumulate diffuse intensity
        totalDiffuse += diff * attenuation;
        
        // Blend light colors additively with attenuation
        combinedLightColor = blendAdditive(combinedLightColor, currentLightColor * diff * attenuation);
    }

    // Blend surface color with light colors (70% light, 30% surface)
    vec3 finalColor = mix(color, combinedLightColor, 0.7);

    // Combine lighting components
    vec3 result = max(totalDiffuse, ambientStrength) * finalColor;

    FragColor = vec4(result, 1.0);
}