#pragma once

#include "particles/behaviours/base_behaviour.hpp"
#include <glm/glm.hpp>

class GravityBehaviour : public BaseBehaviour {
public:
  GravityBehaviour(const glm::vec3 &gravity = glm::vec3(0.0f, -9.81f, 0.0f))
      : gravity(gravity) {}
  virtual ~GravityBehaviour() = default;

  /// Update particle with gravity effect
  inline virtual void update(Particle &particle, float deltaTime,
                             const glm::mat4 &model) override {
    // Apply gravity to velocity
    particle.velocity += gravity * deltaTime;
  }

  /// Set gravity vector
  void setGravity(const glm::vec3 &gravity) { this->gravity = gravity; }

  /// Get gravity vector
  const glm::vec3 &getGravity() const { return gravity; }

private:
  glm::vec3 gravity;
};
