#pragma once

#include "base_emitter.hpp"
#include "graphics/particles/model_particle.hpp"
#include "math/random.hpp"
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>

namespace graphics::particles {

/// Arrow-specific emitter for directional arrow trajectories
class ArrowEmitter : public BaseEmitter {
public:
  /// Constructor
  /// @param initializer Arrow initializer (should be ArrowInitializer or
  /// derived)
  /// @param position Emission position
  /// @param direction Base emission direction (normalized)
  /// @param speed Arrow speed in units per second
  /// @param spreadAngle Spread angle in degrees (0 = straight, 45 = wide cone)
  /// @param parent Parent GameObject for LOCAL/CUSTOM simulation space
  ArrowEmitter(std::shared_ptr<Initializer> initializer,
               // WARN: unused param
               const glm::vec3 &position = glm::vec3{0.0f},
               const glm::vec3 &direction = glm::vec3{0.0f, 0.0f, -1.0f},
               float speed = 10.0f, float spreadAngle = 5.0f,
               const GameObject *parent = nullptr);

  /// Emit an arrow particle
  /// @param particle Particle to emit (will be cast to ModelParticle)
  void emit(Particle &particle) override;

  /// Set emission direction
  /// @param direction New direction (will be normalized)
  void setDirection(const glm::vec3 &direction);

  /// Set arrow speed
  /// @param speed Speed in units per second
  void setSpeed(float speed) { this->speed = speed; }

  /// Set spread angle
  /// @param angle Spread angle in degrees (0 = straight, 45 = wide cone)
  void setSpreadAngle(float angle) { spreadAngle = angle; }

  /// Get current direction
  /// @return Current emission direction
  glm::vec3 getDirection() const { return direction; }

  /// Get current speed
  /// @return Current arrow speed
  float getSpeed() const { return speed; }

  /// Get current spread angle
  /// @return Current spread angle in degrees
  float getSpreadAngle() const { return spreadAngle; }

private:
  float speed;       // Arrow speed in units per second
  float spreadAngle; // Spread angle in degrees

  /// Calculate direction with random spread
  /// @return Direction vector with applied spread
  glm::vec3 calculateSpreadDirection() const;

  /// Generate random point on unit sphere (for spread)
  /// @return Random point on unit sphere
  // glm::vec3 randomPointOnUnitSphere() const;
};

inline ArrowEmitter::ArrowEmitter(std::shared_ptr<Initializer> initializer,
                                  // WARN: unused param
                                  const glm::vec3 &position,
                                  const glm::vec3 &direction, float speed,
                                  float spreadAngle, const GameObject *parent)
    : BaseEmitter{initializer, parent}, speed{speed}, spreadAngle{spreadAngle} {
  // Set position (protected member from BaseEmitter)
  this->position = position;
  setDirection(direction);
}

inline void ArrowEmitter::emit(Particle &particle) {
  // Call base emitter for position calculation
  BaseEmitter::emit(particle);

  // Set initial velocity with spread
  glm::vec3 spreadDir = calculateSpreadDirection();
  particle.velocity = spreadDir * speed;
}

inline void ArrowEmitter::setDirection(const glm::vec3 &direction) {
  if (glm::length(direction) > 0.001f) {
    this->direction = glm::normalize(direction);
  } else {
    this->direction = glm::vec3{0.0f, 0.0f, -1.0f}; // Default forward
  }
}

inline glm::vec3 ArrowEmitter::calculateSpreadDirection() const {
  if (spreadAngle < 0.001f) {
    return direction; // No spread
  }

  // Limit spread to cone around base direction
  // Convert spread angle to radians
  float spreadRad = glm::radians(spreadAngle);

  // Create rotation axis perpendicular to direction
  glm::vec3 axis;
  if (std::abs(direction.x) < 0.1f && std::abs(direction.z) < 0.1f) {
    // Direction is mostly vertical, use different axis
    axis = glm::cross(direction, glm::vec3{1.0f, 0.0f, 0.0f});
  } else {
    axis = glm::cross(direction, glm::vec3{0.0f, 1.0f, 0.0f});
  }

  if (glm::length(axis) < 0.001f) {
    axis = glm::vec3{0.0f, 0.0f, 1.0f};
  }
  axis = glm::normalize(axis);

  // Apply random rotation around axis
  float randomAngle = math::uniformDist() * spreadRad;
  glm::vec3 spreadDir = glm::rotate(direction, randomAngle, axis);

  // Apply additional random rotation around direction axis
  float randomTwist = math::uniformDist() * glm::two_pi<float>();
  spreadDir = glm::rotate(spreadDir, randomTwist, direction);

  return glm::normalize(spreadDir);
}

} // namespace graphics::particles
