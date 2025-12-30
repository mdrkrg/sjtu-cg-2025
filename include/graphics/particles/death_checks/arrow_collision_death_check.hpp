#pragma once

#include "graphics/particles/base/death_check.hpp"
#include "graphics/particles/model_particle.hpp"

namespace graphics::particles {

/// Death check for arrow collision timeout
/// Checks if stuck arrows have exceeded their stuck timeout
class ArrowCollisionDeathCheck : public DeathCheck {
public:
  /// Constructor
  /// @param stuckTimeout Time in seconds before stuck arrows die
  ArrowCollisionDeathCheck(float stuckTimeout = 30.0f)
      : stuckTimeout{stuckTimeout} {}

  /// Only check arrows that are stuck
  bool shouldCheck(const Particle &particle) const override {
    // TODO: Type safety
    const auto &arrow = static_cast<const ModelParticle &>(particle);
    return arrow.stuck;
  }

  /// Check if stuck arrow has timed out
  std::optional<DeathReason> check(const Particle &particle) const override {
    // TODO: Type safety
    const auto &arrow = static_cast<const ModelParticle &>(particle);
    if (arrow.stickTime >= stuckTimeout) {
      return DeathReason::Collision;
    }
    return std::nullopt;
  }

  /// Handle arrow collision death
  void onDeath(Particle &particle, DeathReason reason) override {
    (void)particle;
    (void)reason;
  }

  /// Set stuck timeout
  /// @param timeout New timeout in seconds
  void setStuckTimeout(float timeout) { stuckTimeout = timeout; }

private:
  float stuckTimeout;
};

} // namespace graphics::particles
