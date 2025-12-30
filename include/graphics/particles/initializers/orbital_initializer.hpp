#pragma once

#include "base_initializer.hpp"
#include "graphics/particles/config/orbital_config.hpp"
#include "math/random.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <cmath>

namespace graphics::particles {

/// Particle behaviour for orbital motion around a center point.
class OrbitalParticleInitializer : public BaseInitializer {
public:
  OrbitalParticleInitializer(const OrbitalConfig &config)
      : center(config.center), glowColor(config.glowColor),
        orbitSpeed(config.orbitSpeed), minRadius(config.minRadius),
        maxRadius(config.maxRadius) {}

  virtual ~OrbitalParticleInitializer() = default;

  /// Initialize particle with orbital parameters
  virtual void initialize(Particle &particle) override {
    BaseInitializer::initialize(particle);

    // Random radius within range
    float radius = minRadius + math::uniformDist() * (maxRadius - minRadius);

    // For 3D orbital motion, we need an orbital plane
    // Generate random normal vector for orbital plane
    float theta = 2.0f * glm::pi<float>() * math::uniformDist();
    float phi = glm::acos(2.0f * math::uniformDist() - 1.0f);
    glm::vec3 planeNormal =
        glm::vec3(sinf(phi) * cosf(theta), cosf(phi), sinf(phi) * sinf(theta));

    // Generate random position in the orbital plane
    // Find two orthogonal vectors in the plane
    glm::vec3 u =
        glm::normalize(glm::cross(planeNormal, glm::vec3(0.0f, 1.0f, 0.0f)));
    if (glm::length(u) < 0.001f) {
      u = glm::normalize(glm::cross(planeNormal, glm::vec3(1.0f, 0.0f, 0.0f)));
    }
    glm::vec3 v = glm::normalize(glm::cross(planeNormal, u));

    float angle = 2.0f * glm::pi<float>() * math::uniformDist();
    particle.position = (cosf(angle) * u + sinf(angle) * v) * radius;

    // Tangential velocity (perpendicular to radius vector in the plane)
    glm::vec3 tangent =
        glm::normalize(glm::cross(planeNormal, particle.position));
    particle.velocity = tangent * orbitSpeed * radius;

    // Particle color
    particle.color = glowColor;
    particle.size = 0.04f;

    // HACK: Store radius and plane normal
    // We'll store radius in velocity.y and encode plane normal in velocity
    particle.velocity.y = radius;
    // Store plane normal in a separate variable or encode it
    // For now, we'll recalculate it in update()
  }

  /// Set orbit center
  void setCenter(const glm::vec3 &center) { this->center = center; }

  /// Set orbit speed
  void setOrbitSpeed(float speed) { orbitSpeed = speed; }

private:
  // WARN: not used
  glm::vec3 center;
  glm::vec4 glowColor;
  float orbitSpeed;
  float minRadius;
  float maxRadius;
};
} // namespace graphics::particles
