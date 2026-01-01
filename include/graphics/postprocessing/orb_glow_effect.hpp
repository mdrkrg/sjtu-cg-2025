#pragma once

#include <epoxy/gl.h>
#include <glm/glm.hpp>
#include <string>

#include "screen_space_glow_effect.hpp"
#include "scene/game_object.hpp"

namespace graphics::postprocessing {

/// Orb glow post-processing effect
/// Creates a magical glow around orb objects
class OrbGlowEffect : public ScreenSpaceGlowEffect {
public:
  OrbGlowEffect() = default;
  ~OrbGlowEffect() override = default;

  std::string getName() const override { return "OrbGlowEffect"; }

  /// Enable orb glow for an orb
  void enableForOrb(GameObject *orb, float initialStrength = 1.0f) {
    enableForObject(orb, initialStrength);
    setGlowColor(glm::vec3{0.2, 0.8, 1.0}); // Cyan
    setGlowRadius(0.3f);
    setGlowIntensity(1.0f);
    setGlowFalloffMultiplier(10.0f);
    setPulseBaseIntensity(0.5f);
    setPulseFrequency(2.0f);
  }
};

} // namespace graphics::postprocessing
