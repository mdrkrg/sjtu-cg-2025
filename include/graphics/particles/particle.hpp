#pragma once

#include <glm/glm.hpp>

struct Particle {
  /// Position relative to parent (local space)
  glm::vec3 position;
  /// Velocity in local space
  glm::vec3 velocity;
  glm::vec4 color;
  float life;
  float size;
};

/// The simulation space the particle is in
enum class SimulationSpace {
  /// World space simulation
  WORLD,
  /// Local space simulation
  LOCAL,
  /// Custom space simulation
  CUSTOM,
};
