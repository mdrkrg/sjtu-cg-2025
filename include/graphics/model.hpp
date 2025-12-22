#pragma once

#include <vector>
#include <graphics/mesh.hpp>
#include <graphics/shader.h>

class Model {
public:
  // Stores all textures loaded for this model to prevent duplicates
  // and keep them alive if needed.
  std::vector<Texture> textures_loaded;

  std::vector<Mesh> meshes;

  // Constructor for programmatically created meshes or factory loading
  Model(std::vector<Mesh> &&meshes, std::vector<Texture> &&loadedTextures = {})
      : textures_loaded(std::move(loadedTextures)), meshes(std::move(meshes)) {}

  // Draws the model, and thus all its meshes
  void Draw(Shader &shader) {
    for (unsigned int i = 0; i < meshes.size(); i++)
      meshes[i].Draw(shader);
  }

  // Allow factory to access internals if strictly necessary,
  // though the public constructor handles most needs now.
  friend class ModelFactory;
};
