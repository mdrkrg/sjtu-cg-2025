#version 330 core

#include "include/srgb.glsl"

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D sceneTexture;
uniform vec2 screenSize;

// Debug controls
uniform float exposure = 1.0;

void main() {
    // Sample scene texture (assumed to be in sRGB space)
    vec3 color = texture(sceneTexture, TexCoords).rgb;

    // Convert from sRGB to linear space
    color = sRGBToLinear(color);

    // Apply exposure
    color = color * exposure;

    // Tone mapping (Reinhard)
    color = color / (color + vec3(1.0));

    // Gamma correction (convert back to sRGB for display)
    color = linearToSRGB(color);

    FragColor = vec4(color, 1.0);
}
