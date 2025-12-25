#pragma once

#include "graphics/particles/particle_system.hpp"
#include "graphics/terrain_mesh.hpp"
#include <memory>

class ParticleFactory {
public:
  static std::unique_ptr<ParticleSystem>
  /// Snow particle system in world space (independent particles)
  /// Even if there's a parent (cloud), particles should not follow it
  createRainSystem(std::shared_ptr<TerrainMesh> terrain,
                   const glm::mat4 &terrainModel,
                   const glm::vec3 &position = glm::vec3(0.0f, 10.0f, 0.0f),
                   size_t maxParticles = 1000Z, float emissionRate = 1000.0f,
                   const GameObject *parent = nullptr);

  /// Rain particle system in world space (independent particles)
  /// Even if there's a parent (cloud), particles should not follow it
  static std::unique_ptr<ParticleSystem>
  createSnowSystem(std::shared_ptr<TerrainMesh> terrain,
                   const glm::mat4 &terrainModel,
                   const glm::vec3 &position = glm::vec3(0.0f, 10.0f, 0.0f),
                   size_t maxParticles = 1000Z, float emissionRate = 100.0f,
                   const GameObject *parent = nullptr);
};
