#pragma once

#include "graphics/particles/particle_system.hpp"
#include "graphics/terrain_mesh.hpp"
#include <memory>

namespace graphics::particles {

class ParticleFactory {
public:
  static std::unique_ptr<ParticleSystem<Particle>>
  /// Snow particle system in world space (independent particles)
  /// Even if there's a parent (cloud), particles should not follow it
  createRainSystem(std::shared_ptr<TerrainMesh> terrain,
                   const glm::mat4 &terrainModel,
                   std::shared_ptr<Shader> shader,
                   const glm::vec3 &position = glm::vec3(0.0f, 10.0f, 0.0f),
                   size_t maxParticles = 1000Z, float emissionRate = 1000.0f,
                   const GameObject *parent = nullptr);

  /// Rain particle system in world space (independent particles)
  /// Even if there's a parent (cloud), particles should not follow it
  static std::unique_ptr<ParticleSystem<Particle>>
  createSnowSystem(std::shared_ptr<TerrainMesh> terrain,
                   const glm::mat4 &terrainModel,
                   std::shared_ptr<Shader> shader,
                   const glm::vec3 &position = glm::vec3(0.0f, 10.0f, 0.0f),
                   size_t maxParticles = 1000Z, float emissionRate = 100.0f,
                   const GameObject *parent = nullptr);

  /// Create an orb aura particle system (glowing particles orbiting a center)
  static std::unique_ptr<ParticleSystem<Particle>> createOrbAuraSystem(
      GameObject *const parent, std::shared_ptr<Shader> shader,
      const glm::vec4 &glowColor = glm::vec4(0.2f, 0.8f, 1.0f, 1.0f),
      float intensity = 1.0f, int maxParticles = 1000Z);
};
} // namespace graphics::particles
