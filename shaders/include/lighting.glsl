// Lighting UBO definitions for std140 layout
// Must match C++ structures in include/graphics/light_manager.hpp

#ifndef LIGHTING_GLSL
#define LIGHTING_GLSL

// Maximum number of point lights (must match MAX_POINT_LIGHTS in C++)
#define MAX_POINT_LIGHTS 16

// Point light structure (80 bytes, std140 layout)
struct PointLight {
    vec4 position;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float constant;
    float linear;
    float quadratic;
    float padding;
};

// Lighting data structure for UBO (binding = 1)
layout(std140, binding = 1) uniform LightingData {
    vec4 viewPos;
    PointLight pointLights[MAX_POINT_LIGHTS];
    int numPointLights;
    int padding[3];
} lighting;

#endif // LIGHTING_GLSL
