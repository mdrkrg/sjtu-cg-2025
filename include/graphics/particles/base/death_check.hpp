#pragma once

#include "graphics/particles/particle.hpp"
#include <optional>

namespace graphics::particles {

/// Interface for checking if a particle should die
/// Death checks are registered with ParticleSystem and checked each frame
class DeathCheck {
public:
  virtual ~DeathCheck() = default;

  /// Determine if this death check should be applied to the particle
  /// @param particle Particle to check
  /// @return True if this death check should be evaluated
  virtual bool shouldCheck(const Particle &particle) const = 0;

  /// Check if particle should die
  /// @param particle Particle to check
  /// @return DeathReason if particle should die, nullopt otherwise
  virtual std::optional<DeathReason> check(const Particle &particle) const = 0;

  /// Handle particle death (optional specialized cleanup)
  /// @param particle Particle that died
  /// @param reason Reason for death (the reason this check returned)
  virtual void onDeath(Particle &particle, DeathReason reason) {
    (void)particle;
    (void)reason;
  }
};
} // namespace graphics::particles
