#pragma once

#include "base_behaviour.hpp"
#include "graphics/particles/model_particle.hpp"
#include "math/transform.hpp"
#include <glm/glm.hpp>
#include <glm/gtx/vector_angle.hpp>

namespace graphics::particles {

/// Arrow physics behaviour
/// Extends BaseBehaviour for arrow-specific physics (gravity, drag, rotation)
class ArrowPhysicsBehaviour : public BaseBehaviour {

public:
  /// Constructor
  /// @param gravity Gravity strength (m/s^2)
  /// @param drag Air drag coefficient (0-1)
  /// @param lifetime Maximum lifetime in seconds
  ArrowPhysicsBehaviour(float gravity = 9.81f, float drag = 0.01f,
                        float lifetime = 10.0f)
      : gravity{gravity}, drag{drag}, lifetime{lifetime} {}

  /// Initialize arrow particle
  /// @param particle Particle to initialize
  void initialize(Particle &particle) override {
    BaseBehaviour::initialize(particle);

    auto &arrow = static_cast<ModelParticle &>(particle);

    arrow.life = lifetime; // Use custom lifetime
    arrow.stuck = false;
    arrow.stickTime = 0.0f;
    arrow.rotation = glm::vec3{0.0f};
    arrow.angularVelocity = glm::vec3{0.0f};
  }

  /// Update arrow physics
  /// @param particle Particle to update
  /// @param deltaTime Time since last update
  /// @param model Parent transformation matrix
  /// @param space Simulation space
  void update(Particle &particle, float deltaTime, const glm::mat4 &model,
              SimulationSpace space) override {
    auto &arrow = static_cast<ModelParticle &>(particle);

    // If arrow is stuck, do not update physics
    if (arrow.stuck) {
      arrow.stickTime += deltaTime;
      return;
    }

    // Apply gravity based on simulation space
    if (space == SimulationSpace::WORLD) {
      arrow.velocity.y -= gravity * deltaTime;
    } else if (space == SimulationSpace::LOCAL ||
               space == SimulationSpace::CUSTOM) {
      // Transform gravity to local space
      glm::vec3 localGravity =
          math::transformVecToLocal(glm::vec3{0.0f, -gravity, 0.0f}, model);
      arrow.velocity += localGravity * deltaTime;
    }

    // Apply air drag
    arrow.velocity *= (1.0f - drag * deltaTime);

    // TODO: Update rotation

    // Update life (countdown)
    // arrow.life -= deltaTime;
  }

  /// Using default alive check
  using BaseBehaviour::isAlive;

  /// Handle arrow death (when it hits something or times out)
  /// @param particle Particle that died
  void onDeath(Particle &particle) override {
    auto &arrow = static_cast<ModelParticle &>(particle);
    arrow.reset();
  }

  /// Handle arrow respawn
  /// @param particle Particle being respawned
  void onRespawn(Particle &particle) override { initialize(particle); }

  /// Set gravity strength
  /// @param gravity Gravity in m/s^2
  void setGravity(float gravity) { this->gravity = gravity; }

  /// Set drag coefficient
  /// @param drag Drag coefficient (0-1)
  void setDrag(float drag) { this->drag = drag; }

  /// Set maximum lifetime
  /// @param lifetime Lifetime in seconds
  void setLifetime(float lifetime) { this->lifetime = lifetime; }

private:
  float gravity;  // G force (m/s^2)
  float drag;     // Air drag coefficient
  float lifetime; // Maximum lifetime in seconds
};

} // namespace graphics::particles
