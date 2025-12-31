#version 430 core
out vec4 FragColor;

#include "include/lighting.glsl"
#include "include/material.glsl"

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

uniform Material material;

vec3 blinnPhongAmbient(
    Material material,
    PointLight light,
    vec2 textureUV
) {
  return light.ambient.xyz * material_ambient(material, textureUV);
}

vec3 blinnPhongDiffuse(
    Material material,
    PointLight light,
    vec2 textureUV,
    vec3 lightDir,
    vec3 normDir
) {
    float diff = max(dot(normDir, lightDir), 0.0);
    vec3 textureDiff = material_diffuse(material, textureUV);
    return textureDiff * diff * light.diffuse.xyz;
}

vec3 bisector(vec3 a, vec3 b) {
  vec3 result = a + b;
  return normalize(result);
}

vec3 blinnPhongSpecular(
    Material material,
    PointLight light,
    vec2 textureUV,
    vec3 lightDir,
    vec3 normDir,
    vec3 viewDir
) {
    vec3 bisectorDir = bisector(viewDir, lightDir);
    float angle = dot(normDir, bisectorDir);
    float spec = pow(max(angle, 0.0), material.shininess);
    vec3 textureSpec = material_specular(material, textureUV);
    return light.specular.xyz * spec * textureSpec;
}

vec3 phongSpecular(
    Material material,
    PointLight light,
    vec2 textureUV,
    vec3 lightDir,
    vec3 viewDir,
    vec3 normDir
) {
    vec3 reflectDir = reflect(-lightDir, normDir);
    float angle = dot(viewDir, reflectDir);
    float spec = pow(max(angle, 0.0), material.shininess);
    vec3 textureSpec = material_specular(material, textureUV);
    return light.specular.xyz * spec * textureSpec;
}

vec3 blinnPhong(
    Material material,
    PointLight light,
    vec2 textureUV,
    vec3 lightDir,
    vec3 viewDir,
    vec3 normDir
) {
    vec3 ambient = blinnPhongAmbient(material, light, textureUV);
    vec3 diffuse = blinnPhongDiffuse(material, light, textureUV, lightDir, normDir);
    vec3 specular = blinnPhongSpecular(material, light, textureUV, lightDir, normDir, viewDir);
    vec3 result = diffuse + specular + ambient;
    return result;
}

vec3 phong(
    Material material,
    PointLight light,
    vec2 textureUV,
    vec3 lightDir,
    vec3 viewDir,
    vec3 normDir
) {
    vec3 ambient = blinnPhongAmbient(material, light, textureUV);
    vec3 diffuse = blinnPhongDiffuse(material, light, textureUV, lightDir, normDir);
    vec3 specular = phongSpecular(material, light, textureUV, lightDir, normDir, viewDir);
    vec3 result = diffuse + specular + ambient;
    return result;
}

void main()
{
    vec3 normDir = normalize(Normal);
    vec3 viewDir = normalize(lighting.viewPos.xyz - FragPos);

    vec3 result = vec3(0.0);

    // Process all point lights
    for (int i = 0; i < lighting.numPointLights; ++i) {
        PointLight light = lighting.pointLights[i];

        vec3 lightDir = normalize(light.position.xyz - FragPos);

        // Lighting for this light (no attenuation for room walls)
        vec3 lightContribution = blinnPhong(
            material,
            light,
            TexCoords,
            lightDir,
            viewDir,
            normDir
        );

        result += lightContribution;
    }

    // No lights, use ambient from material
    if (lighting.numPointLights == 0) {
        result = material_ambient(material, TexCoords) * vec3(0.2);
    }

    FragColor = vec4(result, 1.0);
}
