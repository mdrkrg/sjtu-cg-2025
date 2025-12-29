#pragma once

#include "base_updater.hpp"
#include "graphics/particles/model_particle.hpp"
#include "scene/game_object.hpp"
#include <vector>

namespace graphics::particles {

/// AABB-based collision updater for arrows
/// Separate from terrain-specific CollisionUpdater, uses GameObject AABB
/// collision
class ArrowCollisionUpdater : public BaseUpdater {
public:
  /// Constructor
  /// @param collisionTargets List of GameObjects to check for collision
  /// @param collisionMargin Margin around AABB for collision detection
  ArrowCollisionUpdater(const std::vector<GameObject *> &collisionTargets = {},
                        float collisionMargin = 0.01f)
      : collisionTargets{collisionTargets}, collisionMargin{collisionMargin} {}

  /// Update with collision detection
  /// @param particle Particle to update
  /// @param deltaTime Time since last update
  /// @param model Parent transformation matrix
  /// @param space Simulation space
  void update(Particle &particle, float deltaTime, const glm::mat4 &model,
              SimulationSpace space) override {
    BaseUpdater::update(particle, deltaTime, model, space);

    auto &arrow = static_cast<ModelParticle &>(particle);
    // Skip
    if (arrow.stuck) {
      return;
    }

    // Check collision and trigger callback
    for (auto target : collisionTargets) {
      if (not target) {
        continue;
      }

      if (auto result = checkGameObjectCollision(arrow, *target, model);
          result.has_value()) {
        onGameObjectCollision(arrow, *target, result.value());
        break;
      }
    }
  }

  /// Add collision target
  /// @param target GameObject to check for collision
  void addCollisionTarget(GameObject *target) {
    if (!target) {
      return;
    }

    // Check if target already exists
    auto it =
        std::find(collisionTargets.begin(), collisionTargets.end(), target);
    if (it == collisionTargets.end()) {
      collisionTargets.push_back(target);
    }
  }

  /// Remove collision target
  /// @param target GameObject to remove from collision checks
  void removeCollisionTarget(GameObject *target) {
    auto it =
        std::find(collisionTargets.begin(), collisionTargets.end(), target);
    if (it != collisionTargets.end()) {
      collisionTargets.erase(it);
    }
  }

  /// Clear all collision targets
  void clearCollisionTargets() { collisionTargets.clear(); }

  /// Check if particle is alive (override to handle stuck particles)
  /// @param particle Particle to check
  /// @param model Parent transformation matrix
  /// @param space Simulation space
  /// @return True if particle is alive
  bool isAlive(const Particle &particle, const glm::mat4 &model,
               SimulationSpace space) const override {
    const auto &arrow = static_cast<const ModelParticle &>(particle);
    if (arrow.stuck) {
      return arrow.stickTime < STUCK_TIMEOUT;
    }
    return BaseUpdater::isAlive(particle, model, space);
  }

protected:
  struct CollisionResult {
    glm::vec3 point;
    glm::vec3 normal;
  };

  /// Called when arrow collides with a GameObject
  /// @param arrow Arrow particle
  /// @param target GameObject that was hit
  /// @param collisionResult World-space collision result (point and normal)
  virtual void onGameObjectCollision(ModelParticle &arrow, GameObject &target,
                                     const CollisionResult &collisionResult) {
    (void)target;
    const auto &collisionPoint = collisionResult.point;

    { // Arrow is stuck
      arrow.stuck = true;
      arrow.stickTime = 0.0f;
    }

    { // Stop arrow motion
      arrow.velocity = glm::vec3{0.0f};
      arrow.angularVelocity = glm::vec3{0.0f};
    }

    // TODO: Adjust rotation to collisionNormal

    std::println(
        std::cout, "Arrow collided with {} at ({:.6f}, {:.6f}, {:.6f})",
        target.getName(), collisionPoint.x, collisionPoint.y, collisionPoint.z);
  }

private:
  static constexpr float STUCK_TIMEOUT = 30.0f;
  std::vector<GameObject *> collisionTargets;
  float collisionMargin;

  /// Check collision between arrow and GameObject
  /// @param arrow Arrow particle
  /// @param target GameObject to check
  /// @param particleModel Parent transformation matrix
  /// @return CollisionPoint if collision detected, otherwise nullopt
  std::optional<CollisionResult>
  checkGameObjectCollision(const ModelParticle &arrow, const GameObject &target,
                           const glm::mat4 &particleModel) const {

    // Get arrow position in world space
    glm::mat4 arrowTransform = particleModel * arrow.getTransformMatrix();
    glm::vec3 arrowPosWorld =
        glm::vec3{arrowTransform[3]}; // Translation component

    // Get target's world AABB
    scene::AABB targetAABB = target.getWorldAABB();

    // Expand AABB by collision margin
    targetAABB.min -= glm::vec3{collisionMargin};
    targetAABB.max += glm::vec3{collisionMargin};

    // Simple point-in-AABB test (arrow tip collision)
    // TODO: If use full AABB, we need ModelParticleSystem
    // Alternative: ray AABB
    bool collision = targetAABB.wraps(arrowPosWorld);

    if (collision) {
      const auto collisionPoint = arrowPosWorld;
      const auto collisionNormal = targetAABB.closestNormalFrom(arrowPosWorld);
      return CollisionResult{
          std::move(collisionPoint),
          std::move(collisionNormal),
      };
    }

    return std::nullopt;
  }
};
} // namespace graphics::particles
