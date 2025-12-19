#version 430 core

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;
out vec4 FragColor;

#include "include/lighting.glsl"

uniform sampler2D windowTexture;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

uniform Material material;

vec3 blinnPhongAmbient(
    Material material,
    PointLight light
) {
  return material.ambient * light.ambient.xyz;
}

vec3 blinnPhongDiffuse(
    Material material,
    PointLight light,
    vec3 lightDir,
    vec3 normDir
) {
    return material.diffuse * max(dot(normDir, lightDir), 0.0) * light.diffuse.xyz;
}

vec3 bisector(vec3 a, vec3 b) {
  vec3 result = a + b;
  return normalize(result);
}

vec3 blinnPhongSpecular(
    Material material,
    PointLight light,
    vec3 lightDir,
    vec3 normDir,
    vec3 viewDir
) {
    vec3 bisectorDir = bisector(viewDir, lightDir);
    float angle = dot(normDir, bisectorDir);

    float spec = pow(max(angle, 0.0), material.shininess);
    return spec * material.specular * light.specular.xyz;
}

vec3 phongSpecular(
    Material material,
    PointLight light,
    vec3 lightDir,
    vec3 viewDir,
    vec3 normDir
) {
    vec3 reflectDir = reflect(-lightDir, normDir);
    float angle = dot(viewDir, reflectDir);
    float spec = pow(max(angle, 0.0), material.shininess);
    return spec * material.specular * light.specular.xyz;
}

vec3 blinnPhong(
    Material material,
    PointLight light,
    vec3 lightDir,
    vec3 viewDir,
    vec3 normDir
) {
    vec3 ambient = blinnPhongAmbient(material, light);
    vec3 diffuse = blinnPhongDiffuse(material, light, lightDir, normDir);
    vec3 specular = blinnPhongSpecular(material, light, lightDir, normDir, viewDir);
    vec3 result = diffuse + specular + ambient;
    return result;
}

vec3 phong(
    Material material,
    PointLight light,
    vec3 lightDir,
    vec3 viewDir,
    vec3 normDir
) {
    vec3 ambient = blinnPhongAmbient(material, light);
    vec3 diffuse = blinnPhongDiffuse(material, light, lightDir, normDir);
    vec3 specular = phongSpecular(material, light, lightDir, normDir, viewDir);
    vec3 result = diffuse + specular + ambient;
    return result;
}

void main()
{
    vec4 texSample = texture(windowTexture, TexCoords);

    vec3 normDir = normalize(Normal);
    vec3 viewDir = normalize(lighting.viewPos.xyz - FragPos);

    vec3 result = vec3(0.0);

    // Process all point lights
    for (int i = 0; i < lighting.numPointLights; ++i) {
        PointLight light = lighting.pointLights[i];

        vec3 lightDir = normalize(light.position.xyz - FragPos);

        // Lighting for this light (no attenuation for window)
        vec3 lightContribution = blinnPhong(
            material,
            light,
            lightDir,
            viewDir,
            normDir
        );

        result += lightContribution;
    }

    // No lights, use ambient from material
    if (lighting.numPointLights == 0) {
        result = material.ambient * vec3(0.2);
    }

    // Alpha ~1.0: window frame
    // Alpha ~0.2: wall color
    // Alpha ~0.0: transparent
    if (texSample.a > 0.9)
    {
        FragColor = vec4(mix(texSample.rgb, result, 0.2), 1.0);
    }
    else if (texSample.a > 0.1 && texSample.a < 0.3)
    {
        FragColor = vec4(result, 1.0);
    }
    else
    {
        discard;
    }
}
