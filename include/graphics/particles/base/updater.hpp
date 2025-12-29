#pragma once
#include "graphics/particles/particle.hpp"

namespace graphics::particles {

/// Interface of the update behaviour of particles
class Updater {

public:
  /// Update particle state based on behaviour
  virtual void
  update(Particle &particle, float deltaTime, const glm::mat4 &model,
         SimulationSpace space) = 0; // FIXME: This should be const?

  /// Check if particle is still alive
  virtual bool isAlive(const Particle &particle, const glm::mat4 &model,
                       SimulationSpace space) const = 0;

  /// Handle particle death events
  virtual void onDeath(Particle &particle) { (void)particle; };

  /// Handle particle respawn events
  virtual void onRespawn(Particle &particle) { (void)particle; };
};
} // namespace graphics::particles
