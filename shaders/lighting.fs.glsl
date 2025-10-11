#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;

uniform vec3 viewPos;

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
};

uniform Material material;

uniform Light light;


vec3 blinnPhongAmbient(
    Material material,
    Light light
) {
  return material.ambient * light.ambient;
}


vec3 blinnPhongDiffuse(
    Material material,
    Light light,
    vec3 lightDir,
    vec3 normDir
) {
    return material.diffuse * max(dot(normDir, lightDir), 0.0) * light.diffuse;
}


vec3 bisector(vec3 a, vec3 b) {
  vec3 result = a + b;
  return normalize(result);
}


vec3 blinnPhongSpecular(
    Material material,
    Light light,
    vec3 lightDir,
    vec3 normDir,
    vec3 viewDir
) {
    vec3 bisectorDir = bisector(viewDir, lightDir);
    float angle = dot(normDir, bisectorDir);

    float spec = pow(max(angle, 0.0), material.shininess);
    return spec * material.specular * light.specular;
}


vec3 phongSpecular(
    Material material,
    Light light,
    vec3 lightDir,
    vec3 viewDir,
    vec3 normDir
) {
    vec3 reflectDir = reflect(-lightDir, normDir);
    float angle = dot(viewDir, reflectDir);
    float spec = pow(max(angle, 0.0), material.shininess);
    return spec * material.specular * light.specular;
}


vec3 blinnPhong(
    Material material,
    Light light,
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
    Light light,
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
    vec3 normDir = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 result = blinnPhong(
      material,
      light,
      lightDir,
      viewDir,
      normDir
    );
    FragColor = vec4(result, 1.0);
}
