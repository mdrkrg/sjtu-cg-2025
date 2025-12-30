#pragma once

#include "base_updater.hpp"
#include "graphics/particles/config/orbital_config.hpp"
#include "math/random.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <cmath>

namespace graphics::particles {

/// Particle behaviour for orbital motion around a center point.
class OrbitalParticleUpdater : public BaseUpdater {
public:
  OrbitalParticleUpdater(const OrbitalConfig &config)
      : center(config.center), orbitSpeed(config.orbitSpeed),
        minRadius(config.minRadius), maxRadius(config.maxRadius) {}

  virtual ~OrbitalParticleUpdater() = default;

  /// Update orbital motion with centripetal acceleration
  virtual void update(Particle &particle, float deltaTime,
                      const glm::mat4 &model, SimulationSpace space) override {
    // Retrieve radius stored in velocity.y
    float radius = particle.velocity.y;

    // Calculate effective center based on simulation space
    glm::vec3 effectiveCenter = center;
    if (space == SimulationSpace::WORLD) {
      // For WORLD space, center is already in world coordinates
      // No transformation needed
    } else {
      // For LOCAL or CUSTOM space, center should be at origin in local space
      effectiveCenter = glm::vec3(0.0f);
    }

    // Current radius vector from effective center
    glm::vec3 radiusVec = particle.position - effectiveCenter;
    float currentRadius = glm::length(radiusVec);

    // If particle drifted from desired radius, pull it back
    // Use a tolerance to avoid jitter
    const float radiusTolerance = 0.01f;
    if (std::abs(currentRadius - radius) > radiusTolerance &&
        currentRadius > minRadius) {
      glm::vec3 radialDir = radiusVec / currentRadius;
      glm::vec3 desiredPos = effectiveCenter + radialDir * radius;
      particle.position =
          glm::mix(particle.position, desiredPos, 5.0f * deltaTime);
      // Recalculate radiusVec after position adjustment
      radiusVec = particle.position - effectiveCenter;
      currentRadius = glm::length(radiusVec);
    }

    // For 3D orbital motion, we need to maintain tangential velocity
    // perpendicular to the radius vector. We can compute the orbital plane
    // from the current velocity and position.
    if (currentRadius > minRadius) {
      // Normalize radius vector
      glm::vec3 radialDir = radiusVec / currentRadius;

      // Current velocity direction
      glm::vec3 velocityDir = glm::normalize(particle.velocity);

      // Ensure velocity is perpendicular to radius (tangential)
      // Project velocity onto plane perpendicular to radius
      glm::vec3 tangentialVel =
          particle.velocity -
          radialDir * glm::dot(particle.velocity, radialDir);

      // If tangential velocity is too small, create a new tangential direction
      if (glm::length(tangentialVel) < 0.001f) {
        // Create a random vector perpendicular to radius
        glm::vec3 randomVec =
            glm::vec3(math::uniformDist() - 0.5f, math::uniformDist() - 0.5f,
                      math::uniformDist() - 0.5f);
        tangentialVel = glm::normalize(glm::cross(radialDir, randomVec)) *
                        orbitSpeed * radius;
      } else {
        // Normalize and scale to maintain orbital speed
        tangentialVel = glm::normalize(tangentialVel) * orbitSpeed * radius;
      }

      particle.velocity = tangentialVel;
    }

    // Apply velocity
    // FIXME: Should this done outside?
    particle.position += particle.velocity * deltaTime;

    // Pulsating alpha for glow effect
    float pulse = 0.5f + 0.5f * sinf(glm::pi<float>() * particle.life);
    particle.color.a = 0.3f + 0.4f * pulse;
  }

  /// Set orbit center
  void setCenter(const glm::vec3 &center) { this->center = center; }

  /// Set orbit speed
  void setOrbitSpeed(float speed) { orbitSpeed = speed; }

private:
  glm::vec3 center;
  float orbitSpeed;
  float minRadius;
  float maxRadius;
};
} // namespace graphics::particles
