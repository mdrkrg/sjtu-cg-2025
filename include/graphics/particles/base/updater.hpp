#pragma once
#include "graphics/particles/particle.hpp"

namespace graphics::particles {

/// Interface of the update behaviour of particles
/// Handles particle updates and can respond to death events
/// Death checking is now handled separately by DeathCheck instances
class Updater {

public:
  virtual ~Updater() = default;

  /// Update particle state based on behaviour
  virtual void
  update(Particle &particle, float deltaTime, const glm::mat4 &model,
         SimulationSpace space) = 0; // FIXME: This should be const?

  /// Handle particle death events (optional specialized cleanup)
  /// @param particle Particle that died
  /// @param reason Reason for death
  virtual void onDeath(Particle &particle, DeathReason reason) {
    (void)particle;
    (void)reason;
  };

  /// Handle particle respawn events
  virtual void onRespawn(Particle &particle) { (void)particle; };
};
} // namespace graphics::particles
