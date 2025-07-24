#version 330 core

in vec3 color;            // The vertex color (can be used as object's base color)
in vec3 normal;           // The interpolated normal vector at this fragment (world space)
in vec3 currentPosition;  // The interpolated position of the current fragment (world space)

out vec4 FragColor;       // The output color of the fragment

// Uniforms for lighting
uniform vec3 lightColor;      // Color of the light source
uniform vec3 lightPosition;   // Position of the light source (world space)
uniform vec3 cameraPosition;  // Position of the camera (viewer) (world space)
                              // Note: cameraPosition is not strictly needed for just ambient/diffuse,
                              // but kept for consistency if you add other effects later.

void main() {
    // --- Ambient Lighting ---
    // Ambient light is a constant light that illuminates all objects equally,
    // simulating scattered light in the environment.
    float ambientStrength = 0.1; // How strong the ambient light is
    vec3 ambient = ambientStrength * lightColor;

    // --- Diffuse Lighting ---
    // Diffuse light depends on the angle between the light direction and the fragment's normal.
    // It simulates how light reflects off a rough surface.
    vec3 norm = normalize(normal); // Normalize the normal vector (important!)
    vec3 lightDir = normalize(lightPosition - currentPosition); // Direction from fragment to light
    float diff = max(dot(norm, lightDir), 0.0); // Calculate dot product, clamp to 0 if negative
    vec3 diffuse = diff * lightColor;

    // Combine ambient and diffuse lighting components
    vec3 result = (ambient + diffuse) * color; // Multiply by the object's base color

    FragColor = vec4(result, 1.0); // Output the final lit color
}
