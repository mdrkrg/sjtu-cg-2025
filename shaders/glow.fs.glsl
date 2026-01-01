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
uniform vec3 glowColor = vec3(1.0, 1.0, 1.0);
uniform float glowRadius = 1.0;
uniform float glowIntensity = 1.0;
uniform float glowFalloffMultiplier = 10.0;

uniform float pulseBaseIntensity = 0.5;
uniform float pulseFrequency = 1.0;

void main() {
    vec3 color = texture(sceneTexture, TexCoords).rgb;

    vec2 uv = TexCoords;
    float distanceToGlow = distance(uv, glowScreenPos);

    // Falloff
    float glowFactor = exp(-distanceToGlow * glowFalloffMultiplier / glowRadius);

    // Pulsing
    float pulse = pulseBaseIntensity + (1 - pulseBaseIntensity) * sin(time * pulseFrequency);
    glowFactor *= pulse * effectStrength * glowIntensity;

    vec3 glow = glowColor * glowFactor;
    color += glow;

    // Clamp
    color = min(color, vec3(1.0));

    FragColor = vec4(color, 1.0);
}
