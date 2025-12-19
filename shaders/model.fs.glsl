#version 430 core
out vec4 FragColor;

#include "include/lighting.glsl"

struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    sampler2D texture_normal1;
    sampler2D texture_height1;
    float shininess;
    bool emissive;
    vec3 emission;
    float emissionStrength;
};

in vec3 FragPos;
in vec2 TexCoords;

uniform Material material;

vec3 blinnPhongAmbient(
    Material material,
    PointLight light,
    vec2 textureUV
) {
    // NOTE: texture map as ambient
    return light.ambient.xyz * texture(material.texture_diffuse1, textureUV).rgb;
}

vec3 blinnPhongDiffuse(
    Material material,
    PointLight light,
    vec2 textureUV,
    vec3 lightDir,
    vec3 normDir
) {
    float diff = max(dot(normDir, lightDir), 0.0);
    // NOTE: texture map as diffuse
    vec3 textureDiff = texture(material.texture_diffuse1, textureUV).rgb;
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
    vec3 textureSpec = texture(material.texture_specular1, textureUV).rgb;
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
    vec3 textureSpec = texture(material.texture_specular1, textureUV).rgb;
    return spec * textureSpec * light.specular.xyz;
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
    vec3 specular = blinnPhongSpecular(material, light, textureUV, lightDir, normDir, viewDir);
    vec3 result = diffuse + specular + ambient;
    return result;
}

void main()
{
    vec3 normDir = normalize(texture(material.texture_normal1, TexCoords).rgb);
    vec3 viewDir = normalize(lighting.viewPos.xyz - FragPos);

    vec3 result = vec3(0.0);

    // Process all point lights
    for (int i = 0; i < lighting.numPointLights; ++i) {
        PointLight light = lighting.pointLights[i];

        vec3 lightDir = normalize(light.position.xyz - FragPos);
        float distance = length(light.position.xyz - FragPos);
        float attenuation = 1.0 / (light.constant + light.linear * distance +
                                 light.quadratic * (distance * distance));

        // Lighting for this light
        vec3 lightContribution = blinnPhong(
            material,
            light,
            TexCoords,
            lightDir,
            viewDir,
            normDir
        );

        result += lightContribution * attenuation;
    }

    // No lights, use ambient from material
    if (lighting.numPointLights == 0) {
        result = texture(material.texture_diffuse1, TexCoords).rgb * vec3(0.2);
    }

    // Add emission if enabled
    if (material.emissive) {
        result += material.emission * material.emissionStrength;
    }

    FragColor = vec4(result, 1.0);
}
