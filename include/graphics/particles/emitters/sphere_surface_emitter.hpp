#pragma once

#include "base_emitter.hpp"
#include "math/random.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <cmath>

namespace graphics::particles {

/// Emitter that emits particles from the surface of a sphere.
class SphereSurfaceEmitter : public BaseEmitter {
public:
  SphereSurfaceEmitter(std::shared_ptr<Initializer> initializer,
                       GameObject *obj = nullptr, float radius = 1.0f)
      : BaseEmitter{initializer, obj, SimulationSpace::WORLD}, radius{radius} {}
  virtual ~SphereSurfaceEmitter() = default;

  /// Emit a particle from a random point on sphere surface
  inline virtual void emit(Particle &particle) override {
    BaseEmitter::emit(particle);

    // Generate random point on sphere surface using spherical coordinates
    float theta = 2.0f * glm::pi<float>() * math::uniformDist(); // [0, 2pi]
    float phi = glm::acos(2.0f * math::uniformDist() - 1.0f);    // [0, pi]

    // Convert to Cartesian coordinates (local shape position)
    const auto x = radius * sinf(phi) * cosf(theta);
    const auto y = radius * cosf(phi);
    const auto z = radius * sinf(phi) * sinf(theta);
    const glm::vec3 localPos = glm::vec3(x, y, z);

    // Apply simulation space transformation
    particle.position = calculatePositionBySimulationSpace(localPos);

    // Optional: small outward velocity (in local space)
    const auto normal = glm::normalize(localPos);
    particle.velocity = normal * 0.1f;

    // Transform velocity to world space if needed
    if (simulationSpace == SimulationSpace::WORLD && parent) {
      const auto parentRotation = glm::mat3(parent->getModelMatrix());
      particle.velocity = parentRotation * particle.velocity;
    }
  }

  /// Set sphere radius
  void setRadius(float radius) { this->radius = radius; }

  /// Get sphere radius
  float getRadius() const { return radius; }

private:
  float radius;
};
} // namespace graphics::particles
