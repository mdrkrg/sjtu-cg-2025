#pragma once

#include "particle.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace graphics::particles {

/// Model-based particle extending the base Particle struct
/// Adds rotation and collision state for model rendering
struct ModelParticle : public Particle {
  /// Euler rotation angles in degrees (X, Y, Z)
  glm::vec3 rotation{0.0f, 0.0f, 0.0f};

  /// Rotation speed in degrees per second
  glm::vec3 angularVelocity{0.0f, 0.0f, 0.0f};

  /// Whether the particle is stuck to a collision surface
  bool stuck{false};

  /// Time since the particle became stuck
  float stickTime{0.0f};

  /// Default constructor
  ModelParticle() = default;

  /// Constructor from base Particle
  explicit ModelParticle(const Particle &base)
      : Particle{base}, rotation{0.0f, 0.0f, 0.0f},
        angularVelocity{0.0f, 0.0f, 0.0f}, stuck{false}, stickTime{0.0f} {}

  /// Get transformation matrix for this particle
  /// @param parentMatrix Parent transformation matrix (from ParticleSystem)
  /// @return Combined transformation matrix
  glm::mat4 getTransformMatrix(const glm::mat4 &parentMatrix = glm::mat4{
                                   1.0f}) const {
    glm::mat4 transform = parentMatrix;

    // Translate to particle position
    transform = glm::translate(transform, position);

    // Apply rotation (Euler angles in degrees)
    // Note: glm::eulerAngles returns angles for ZYX rotation order
    // So we apply rotations in Z, Y, X order (roll, yaw, pitch)
    transform = glm::rotate(transform, glm::radians(rotation.z),
                            glm::vec3{0.0f, 0.0f, 1.0f});
    transform = glm::rotate(transform, glm::radians(rotation.y),
                            glm::vec3{0.0f, 1.0f, 0.0f});
    transform = glm::rotate(transform, glm::radians(rotation.x),
                            glm::vec3{1.0f, 0.0f, 0.0f});

    // Apply scale (using size for uniform scaling)
    transform = glm::scale(transform, glm::vec3{size});

    return transform;
  }

  /// Update model particle state
  /// @param deltaTime Time since last update in seconds
  void update(float deltaTime) {
    // Position and life is updated via base particle
    if (!stuck) {
      // Update rotation based on angular velocity
      rotation += angularVelocity * deltaTime;
    } else {
      // Update stick time for stuck particles
      stickTime += deltaTime;
    }
  }

  /// Reset particle to default state
  void reset() {
    position = glm::vec3{0.0f};
    velocity = glm::vec3{0.0f};
    color = glm::vec4{1.0f};
    life = 0.0f;
    size = 1.0f;
    rotation = glm::vec3{0.0f};
    angularVelocity = glm::vec3{0.0f};
    stuck = false;
    stickTime = 0.0f;
  }
};

} // namespace graphics::particles
