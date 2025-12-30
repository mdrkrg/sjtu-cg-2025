#pragma once

#include "graphics/particles/base/death_check.hpp"

namespace graphics::particles {

/// Universal death check for particle lifetime timeout
/// Always checks all particles, returns DeathReason::Timeout when life <= 0
class TimeoutDeathCheck : public DeathCheck {
public:
  TimeoutDeathCheck() = default;

  /// Always check timeout for all particles
  bool shouldCheck(const Particle &particle) const override {
    (void)particle;
    return true;
  }

  /// Check if particle life has expired
  std::optional<DeathReason> check(const Particle &particle) const override {
    if (particle.life <= 0.0f) {
      return DeathReason::Timeout;
    }
    return std::nullopt;
  }
};
} // namespace graphics::particles
