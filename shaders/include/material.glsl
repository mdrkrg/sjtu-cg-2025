#ifndef MATERIAL_GLSL
#define MATERIAL_GLSL

struct Material {
  sampler2D texture_diffuse1;
  sampler2D texture_specular1;
  sampler2D texture_normal1;
  sampler2D texture_height1;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float shininess;
  bool emissive;
  vec3 emission;
  float emissionStrength;
  bool use_texture;
};

vec3 material_ambient(Material material, vec2 uv) {
  return material.use_texture
    // NOTE: texture map as diffuse
    ? texture(material.texture_diffuse1, uv).rgb
    : material.ambient;
}

vec3 material_diffuse(Material material, vec2 uv) {
  return material.use_texture
    ? texture(material.texture_diffuse1, uv).rgb
    : material.diffuse;
}

vec3 material_specular(Material material, vec2 uv) {
  return material.use_texture
    ? texture(material.texture_specular1, uv).rgb
    : material.specular;
}

#endif // MATERIAL_GLSL
