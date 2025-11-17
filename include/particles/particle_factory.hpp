#pragma once

#include "particles/particle_system.hpp"
#include "terrain_mesh.hpp"
#include <memory>

class ParticleFactory {
public:
  static std::unique_ptr<ParticleSystem>
  createRainSystem(std::shared_ptr<TerrainMesh> terrain,
                   const glm::mat4 &terrainModel,
                   const glm::vec3 &position = glm::vec3(0.0f, 10.0f, 0.0f),
                   size_t maxParticles = 1000Z, float emissionRate = 1000.0f);

  static std::unique_ptr<ParticleSystem>
  createSnowSystem(std::shared_ptr<TerrainMesh> terrain,
                   const glm::mat4 &terrainModel,
                   const glm::vec3 &position = glm::vec3(0.0f, 10.0f, 0.0f),
                   size_t maxParticles = 1000Z, float emissionRate = 100.0f);
};
