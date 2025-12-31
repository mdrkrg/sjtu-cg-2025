#pragma once

#include "base_initializer.hpp"
#include "graphics/particles/model_particle.hpp"
#include <glm/glm.hpp>
#include <glm/gtx/vector_angle.hpp>

namespace graphics::particles {

/// Arrow initializer
/// Extends BaseInitializer for arrow-specific physics
class ArrowInitializer : public BaseInitializer {

public:
  /// Constructor
  /// @param lifetime Maximum lifetime in seconds
  ArrowInitializer(float lifetime = 10.0f) : lifetime{lifetime} {}

  /// Initialize arrow particle
  /// @param particle Particle to initialize
  void initialize(Particle &particle) override {
    BaseInitializer::initialize(particle);

    particle.life = lifetime;
    particle.color = glm::vec4{0.8f, 0.6f, 0.2f, 1.0f}; // Bronze color
    // TODO: parameterize this
    particle.size = 0.2f;

    // TODO: Safe type casting
    auto &arrow = static_cast<ModelParticle &>(particle);

    arrow.stuck = false;
    arrow.stickTime = 0.0f;

    initRotationFromVelocity(arrow);

    arrow.angularVelocity = glm::vec3{0.0f};
  }

  /// Set maximum lifetime
  /// @param lifetime Lifetime in seconds
  void setLifetime(float lifetime) { this->lifetime = lifetime; }

private:
  float lifetime; // Maximum lifetime in seconds

  /// Set initial rotation based on velocity direction
  void initRotationFromVelocity(ModelParticle &arrow) {
    // Arrow points along +Y initially
    static constexpr glm::vec3 modelForward{0.0f, 1.0f, 0.0f};

    if (glm::length2(arrow.velocity) > 0.000001f) {
      const auto direction = glm::normalize(arrow.velocity);
      const auto q = glm::rotation(modelForward, direction);
      // Convert to degrees
      arrow.rotation = glm::degrees(glm::eulerAngles(q));
    } else {
      // Default to up
      arrow.rotation = glm::vec3{0.0f, 0.0f, 0.0f};
    }
  }
};
} // namespace graphics::particles
