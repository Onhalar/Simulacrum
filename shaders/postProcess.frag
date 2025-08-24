#version 330 core
out vec4 FragColor;

in vec2 textureUV;

uniform sampler2D screenTexture;

void main()
{
    //FragColor = texture(screenTexture, textureUV); // - Normal
    FragColor = FragColor = vec4(vec3(1.0 - texture(screenTexture, textureUV)), 1.0); // - Inverted

    
    // Grayscale
    // vec3 color = texture(screenTexture, TexCoords).rgb;
    // float average = (color.r + color.g + color.b) / 3.0;
    // FragColor = vec4(average, average, average, 1.0);
}