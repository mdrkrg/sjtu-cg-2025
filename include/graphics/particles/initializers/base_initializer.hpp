#pragma once

#include "graphics/particles/base/initializer.hpp"

namespace graphics::particles {

class BaseInitializer : public Initializer {
public:
  BaseInitializer() = default;
  virtual ~BaseInitializer() = default;

  /// Default implementation for particle initialization
  virtual void initialize(Particle &particle) override {
    particle.position = glm::vec3(0.0f);
    particle.velocity = glm::vec3(0.0f);
    particle.color = glm::vec4(1.0f);
    particle.life = 1.0f;
    particle.size = 1.0f;
  }
};
} // namespace graphics::particles
