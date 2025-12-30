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

  /// Check if particle is alive
  /// @return True if particle has life remaining
  bool alive() const { return life > 0.0f; }
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

/// Reason for particle death
enum class DeathReason {
  /// Life expired
  Timeout,
  /// Collided with something
  Collision,
  /// Left visible area
  OutOfBounds,
  /// Animation finished
  Dissolved,
  /// Other reasons
  Custom
};
