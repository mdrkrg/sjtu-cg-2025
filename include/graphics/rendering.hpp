#pragma once

#include "graphics/material.hpp"
#include "graphics/mesh.hpp"

namespace graphics {

inline void setMaterialToShader(const Shader &shader,
                                const Material &material) {
  shader.setVec3("material.ambient", material.ambient);
  shader.setVec3("material.diffuse", material.diffuse);
  shader.setVec3("material.specular", material.specular);
  shader.setFloat("material.shininess", material.shininess);
  // For textured materials, shader uses textures from model
  shader.setBool("material.use_texture", material.usesTextures());

  // Set emission if enabled
  if (material.isEmissive()) {
    shader.setBool("material.emissive", true);
    shader.setVec3("material.emission", material.emissionColor);
    shader.setFloat("material.emissionStrength", material.emissionStrength);
  } else {
    shader.setBool("material.emissive", false);
  }
}

/// Render a mesh with material and shader
inline void renderMesh(const Mesh &mesh, const Material &material,
                       const Shader &shader) {

  shader.use();
  setMaterialToShader(shader, material);
  mesh.Draw(shader);
}

/// Render a mesh with material and shader with instanced randering
inline void renderMeshInstanced(const Mesh &mesh, const Material &material,
                                const Shader &shader, size_t instanceCount) {

  shader.use();
  setMaterialToShader(shader, material);
  mesh.DrawInstanced(shader, instanceCount);
}

} // namespace graphics
