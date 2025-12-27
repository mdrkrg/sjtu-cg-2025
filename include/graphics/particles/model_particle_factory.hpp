#pragma once

#include "model_particle_system.hpp"
#include <memory>

namespace graphics::particles {

/// Factory for creating model-based particle systems
class ModelParticleFactory {
public:
  /// Create a simple test cube particle system
  /// @param shader Shader for the particle system
  /// @param position Emission position
  /// @param maxParticles Maximum number of particles
  /// @param emissionRate Particles per second
  /// @param cubeSize Size of each cube
  /// @param cubeColor Color of cubes
  /// @return ModelParticleSystem with falling cubes
  static std::shared_ptr<ModelParticleSystem> createCubeTestSystem(
      std::shared_ptr<Shader> shader,
      const glm::vec3 &position = glm::vec3{0.0f, 0.0f, 0.0f},
      size_t maxParticles = 50, float emissionRate = 2.0f,
      float cubeSize = 0.1f,
      const glm::vec3 &cubeColor = glm::vec3{1.0f, 1.0f, 1.0f});
};
} // namespace graphics::particles
