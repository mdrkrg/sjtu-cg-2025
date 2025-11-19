#version 330 core
out vec4 FragColor;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};


struct Light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

uniform vec3 viewPos;
uniform Material material;
uniform Light light;


vec3 blinnPhongAmbient(
    Material material,
    Light light,
    vec2 textureUV
) {
    // NOTE: texture map as ambient
    return light.ambient * material.diffuse;
}


vec3 blinnPhongDiffuse(
    Material material,
    Light light,
    vec2 textureUV,
    vec3 lightDir,
    vec3 normDir
) {
    float diff = max(dot(normDir, lightDir), 0.0);
    // NOTE: texture map as diffuse
    return material.diffuse * diff * light.diffuse;
}


vec3 bisector(vec3 a, vec3 b) {
  vec3 result = a + b;
  return normalize(result);
}


vec3 blinnPhongSpecular(
    Material material,
    Light light,
    vec2 textureUV,
    vec3 lightDir,
    vec3 normDir,
    vec3 viewDir
) {
    vec3 bisectorDir = bisector(viewDir, lightDir);
    float angle = dot(normDir, bisectorDir);
    float spec = pow(max(angle, 0.0), material.shininess);
    return light.specular * spec * material.specular;
}


vec3 phongSpecular(
    Material material,
    Light light,
    vec2 textureUV,
    vec3 lightDir,
    vec3 viewDir,
    vec3 normDir
) {
    vec3 reflectDir = reflect(-lightDir, normDir);
    float angle = dot(viewDir, reflectDir);
    float spec = pow(max(angle, 0.0), material.shininess);
    return light.specular * spec * material.specular;
}


vec3 blinnPhong(
    Material material,
    Light light,
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
    Light light,
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
    // diffuse
    vec3 normDir = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float lightDistance = length(light.position - FragPos);

    float attenuation = 1.0 / (light.constant + light.linear * lightDistance + light.quadratic * (lightDistance * lightDistance));

    // specular
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 result = blinnPhong(
      material,
      light,
      TexCoords,
      lightDir,
      viewDir,
      normDir
    );

    FragColor = vec4(result, 1.0);
}

