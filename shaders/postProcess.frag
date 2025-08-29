#version 330 core
out vec4 FragColor;

in vec2 textureUV;

uniform sampler2D screenTexture;

uniform vec2 resolution;
uniform bool enableFXAA = true;
uniform bool inverseColors = false;

// FXAA Constants
#define FXAA_REDUCE_MIN (1.0/128.0)
#define FXAA_REDUCE_MUL (1.0/8.0)
#define FXAA_SPAN_MAX 8.0

vec4 applyFXAA(sampler2D tex, vec2 uv, vec2 res) {
    // FXAA implementation based on work by Timothy Lottes (NVIDIA)
    // Reference: GPU Gems 3, Chapter 13 - "Fast Approximate Anti-Aliasing"
    // directly adapted from: https://github.com/McNopper/OpenGL/blob/master/Example42/shader/fxaa.frag.glsl (under Apache 2.0 license - 2004) from Github user McNopper

    vec2 inverseRes = 1.0 / res;
    
    vec3 rgbNW = texture(tex, uv + vec2(-1.0, -1.0) * inverseRes).xyz;
    vec3 rgbNE = texture(tex, uv + vec2(1.0, -1.0) * inverseRes).xyz;
    vec3 rgbSW = texture(tex, uv + vec2(-1.0, 1.0) * inverseRes).xyz;
    vec3 rgbSE = texture(tex, uv + vec2(1.0, 1.0) * inverseRes).xyz;
    vec3 rgbM = texture(tex, uv).xyz;
    
    vec3 luma = vec3(0.299, 0.587, 0.114);
    float lumaNW = dot(rgbNW, luma);
    float lumaNE = dot(rgbNE, luma);
    float lumaSW = dot(rgbSW, luma);
    float lumaSE = dot(rgbSE, luma);
    float lumaM = dot(rgbM, luma);
    
    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));
    
    vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y = ((lumaNW + lumaSW) - (lumaNE + lumaSE));
    
    float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);
    float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
    dir = min(vec2(FXAA_SPAN_MAX, FXAA_SPAN_MAX), max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX), dir * rcpDirMin)) * inverseRes;
    
    vec3 rgbA = 0.5 * (
        texture(tex, uv + dir * (1.0/3.0 - 0.5)).xyz +
        texture(tex, uv + dir * (2.0/3.0 - 0.5)).xyz);
    
    vec3 rgbB = rgbA * 0.5 + 0.25 * (
        texture(tex, uv + dir * -0.5).xyz +
        texture(tex, uv + dir * 0.5).xyz);
    
    float lumaB = dot(rgbB, luma);
    
    if ((lumaB < lumaMin) || (lumaB > lumaMax)) {
        return vec4(rgbA, 1.0);
    } else {
        return vec4(rgbB, 1.0);
    }
}

void main() {
    vec4 outColor = texture(screenTexture, textureUV);

    if (enableFXAA) { outColor = applyFXAA(screenTexture, textureUV, resolution); }

    if (inverseColors) { outColor = vec4(1.0 - outColor.rgb, 1.0); }

    FragColor = outColor;
}