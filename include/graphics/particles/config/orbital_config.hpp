#pragma once
#include <glm/glm.hpp>

namespace graphics::particles {

struct OrbitalConfig {
  const glm::vec3 &center;
  const glm::vec4 &glowColor = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
  float orbitSpeed = 1.0f;
  float minRadius = 0.5f;
  float maxRadius = 1.5f;
};
} // namespace graphics::particles
