#pragma once

#include "base_behaviour.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <cmath>

/// Particle behaviour for orbital motion around a center point.
class OrbitalParticleBehaviour : public BaseBehaviour {
public:
  OrbitalParticleBehaviour(const glm::vec3 &center,
                           const glm::vec4 &glowColor = glm::vec4{1.0f, 1.0f,
                                                                  1.0f, 1.0f},
                           float orbitSpeed = 1.0f, float minRadius = 0.5f,
                           float maxRadius = 1.5f)
      : center(center), glowColor(glowColor), orbitSpeed(orbitSpeed),
        minRadius(minRadius), maxRadius(maxRadius) {}

  virtual ~OrbitalParticleBehaviour() = default;

  /// Initialize particle with orbital parameters
  virtual void initialize(Particle &particle) override {
    BaseBehaviour::initialize(particle);

    // Random radius within range
    float radius = minRadius + uniformDist(gen) * (maxRadius - minRadius);

    // For 3D orbital motion, we need an orbital plane
    // Generate random normal vector for orbital plane
    float theta = 2.0f * glm::pi<float>() * uniformDist(gen);
    float phi = glm::acos(2.0f * uniformDist(gen) - 1.0f);
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

    float angle = 2.0f * glm::pi<float>() * uniformDist(gen);
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
            glm::vec3(uniformDist(gen) - 0.5f, uniformDist(gen) - 0.5f,
                      uniformDist(gen) - 0.5f);
        tangentialVel = glm::normalize(glm::cross(radialDir, randomVec)) *
                        orbitSpeed * radius;
      } else {
        // Normalize and scale to maintain orbital speed
        tangentialVel = glm::normalize(tangentialVel) * orbitSpeed * radius;
      }

      particle.velocity = tangentialVel;
    }

    // Apply velocity
    particle.position += particle.velocity * deltaTime;

    // Update lifetime (fade in/out)
    particle.life -= deltaTime * 0.5f; // Longer lifetime
    if (particle.life <= 0.0f) {
      particle.life = 1.0f; // respawn
      initialize(particle);
    }

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
  glm::vec4 glowColor;
  float orbitSpeed;
  float minRadius;
  float maxRadius;
};
