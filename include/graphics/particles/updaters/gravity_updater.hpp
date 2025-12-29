#pragma once
#include "graphics/particles/updaters/base_updater.hpp"
#include "math/transform.hpp"

namespace graphics::particles {

class GravityUpdater : public BaseUpdater {
public:
  GravityUpdater(const glm::vec3 &gravity = glm::vec3(0.0f, -9.81f, 0.0f))
      : gravity(gravity) {}

  virtual ~GravityUpdater() = default;

  /// Update particle with gravity effect
  inline virtual void update(Particle &particle, float deltaTime,
                             const glm::mat4 &model,
                             SimulationSpace space) override {
    // Apply gravity to velocity based on simulation space
    if (space == SimulationSpace::WORLD) {
      // WORLD space: apply gravity directly
      particle.velocity += gravity * deltaTime;
    } else {
      // Space is not world, transform gravity to local space
      const auto localGravity =
          math::transformVecToLocal(gravity, glm::mat3{model});
      particle.velocity += localGravity * deltaTime;
    }
  }

  /// Set gravity vector
  void setGravity(const glm::vec3 &gravity) { this->gravity = gravity; }

  /// Get gravity vector
  const glm::vec3 &getGravity() const { return gravity; }

private:
  glm::vec3 gravity;
};

} // namespace graphics::particles
