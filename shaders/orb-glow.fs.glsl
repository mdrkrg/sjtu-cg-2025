#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

// Screen texture in sRGB
uniform sampler2D sceneTexture;
uniform vec2 screenSize;
uniform vec2 glowScreenPos;
uniform float effectStrength;
uniform float time;

// Glow parameters
uniform vec3 glowColor = vec3(0.2, 0.8, 1.0); // Cyan
uniform float glowRadius = 0.3;
uniform float glowIntensity = 1.0;

void main() {
    vec3 color = texture(sceneTexture, TexCoords).rgb;

    vec2 uv = TexCoords;
    float distanceToGlow = distance(uv, glowScreenPos);

    // Falloff
    float glowFactor = exp(-distanceToGlow * 10.0 / glowRadius);

    // Pulsing
    float pulse = 0.5 + 0.5 * sin(time * 2.0);
    glowFactor *= pulse * effectStrength * glowIntensity;

    vec3 glow = glowColor * glowFactor;
    color += glow;

    // Clamp
    color = min(color, vec3(1.0));

    FragColor = vec4(color, 1.0);
}
