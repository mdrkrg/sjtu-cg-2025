#pragma once

#include <epoxy/gl.h>
#include <glm/glm.hpp>
#include <string>

#include "screen_space_glow_effect.hpp"
#include "scene/game_object.hpp"

namespace graphics::postprocessing {

/// Lamp aura post-processing effect
/// Creates a warm glow around lamp objects when they're turned on
class LampAuraEffect : public ScreenSpaceGlowEffect {
public:
  LampAuraEffect() = default;
  ~LampAuraEffect() override = default;

  std::string getName() const override { return "LampAuraEffect"; }

  /// Enable lamp aura for a lamp
  void enableForLamp(GameObject *lamp, float initialStrength = 1.0f) {
    enableForObject(lamp, initialStrength);

    setGlowColor(glm::vec3{1.0f, 0.8f, 0.4f}); // Yellow
    setGlowRadius(0.4f);
    setGlowIntensity(0.8f);
    setGlowFalloffMultiplier(8.0f);
    setPulseBaseIntensity(0.7f);
    setPulseFrequency(1.5f);
  }
};

} // namespace graphics::postprocessing
