#pragma once

#include "graphics/particles/base/updater.hpp"
#include <glm/glm.hpp>

namespace graphics::particles {

class BaseUpdater : public Updater {
public:
  BaseUpdater() = default;
  virtual ~BaseUpdater() = default;

  /// Default implementation for particle update
  virtual void update(Particle &particle, float deltaTime,
                      const glm::mat4 &model, SimulationSpace space) override {
    // Empty, subclasses can override
    (void)particle;
    (void)deltaTime;
    (void)model;
    (void)space;
  }

  /// Default implementation for alive check
  virtual bool isAlive(const Particle &particle, const glm::mat4 &model,
                       SimulationSpace space) const override {
    (void)model;
    (void)space;
    return particle.life > 0.0f;
  }
};
} // namespace graphics::particles
