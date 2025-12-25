#pragma once

#include "graphics/particles/behaviours/base_behaviour.hpp"
#include "math/transform.hpp"
#include <glm/glm.hpp>

class WindBehaviour : public BaseBehaviour {
public:
  WindBehaviour(const glm::vec3 &windForce = glm::vec3(0.0f),
                float turbulence = 0.0f)
      : windForce(windForce), turbulence(turbulence) {}
  virtual ~WindBehaviour() = default;

  /// Update particle with wind effect
  inline virtual void update(Particle &particle, float deltaTime,
                             const glm::mat4 &model,
                             SimulationSpace space) override {
    // Apply wind force based on simulation space
    if (space == SimulationSpace::WORLD) {
      // WORLD space: apply wind force directly
      particle.velocity += windForce * deltaTime;
    } else {
      // Space is not world, transform wind force to local space
      const auto localWindForce =
          math::transformVecToLocal(windForce, glm::mat3{model});
      particle.velocity += localWindForce * deltaTime;
    }

    // Turbulence (always in local space)
    if (turbulence > 0.0f) {
      float turbulenceX = (uniformDist(gen) - 0.5f) * 2.0f * turbulence;
      float turbulenceZ = (uniformDist(gen) - 0.5f) * 2.0f * turbulence;
      particle.velocity.x += turbulenceX * deltaTime;
      particle.velocity.z += turbulenceZ * deltaTime;
    }
  }

  /// Set wind force
  void setWindForce(const glm::vec3 &windForce) { this->windForce = windForce; }

  /// Set turbulence factor
  void setTurbulence(float turbulence) { this->turbulence = turbulence; }

private:
  glm::vec3 windForce;
  float turbulence;
};
