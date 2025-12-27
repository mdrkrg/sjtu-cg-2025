#pragma once

#include "base_behaviour.hpp"
#include "graphics/particles/model_particle.hpp"
#include "graphics/particles/model_particle_system.hpp"
#include "scene/game_object.hpp"
#include <vector>

namespace graphics::particles {

/// AABB-AABB collision behaviour for ModelParticles
/// Checks collision between ModelParticle AABB and GameObject AABB
class ModelParticleAABBCollisionBehaviour : public BaseBehaviour {
public:
  /// Constructor
  /// @param particleSystem ModelParticleSystem reference (for particle local
  /// AABB)
  /// @param collisionTargets List of GameObjects to check for collision
  /// @param collisionMargin Margin around AABBs for collision detection
  ModelParticleAABBCollisionBehaviour(
      ModelParticleSystem *particleSystem,
      const std::vector<GameObject *> &collisionTargets = {},
      float collisionMargin = 0.01f)
      : particleSystem{particleSystem}, collisionTargets{collisionTargets},
        collisionMargin{collisionMargin} {
    if (!particleSystem) {
      std::cerr << "Warning: ModelParticleAABBCollisionBehaviour created with "
                   "null particleSystem"
                << std::endl;
    }
  }

  /// Update with AABB-AABB collision detection
  /// @param particle Particle to update (must be ModelParticle)
  /// @param deltaTime Time since last update
  /// @param model Parent transformation matrix (particle system model)
  /// @param space Simulation space
  void update(Particle &particle, float deltaTime, const glm::mat4 &model,
              SimulationSpace space) override;

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
  void clearCollisionTargets() { collisionTargets.clear(); };

  /// Set collision margin
  /// @param margin Margin around AABBs for collision detection
  void setCollisionMargin(float margin) { collisionMargin = margin; }

  /// Get collision margin
  /// @return Current collision margin
  float getCollisionMargin() const { return collisionMargin; }

  /// Check if particle is alive (override to handle collision termination)
  /// @param particle Particle to check
  /// @param model Parent transformation matrix
  /// @param space Simulation space
  /// @return True if particle is alive
  bool isAlive(const Particle &particle, const glm::mat4 &model,
               SimulationSpace space) const override {
    return BaseBehaviour::isAlive(particle, model, space);
  }

protected:
  /// Called when ModelParticle collides with a GameObject
  /// @param particle ModelParticle that collided
  /// @param target GameObject that was hit
  /// @param collisionAABB Overlapping AABB region (for debugging)
  virtual void onGameObjectCollision(ModelParticle &particle,
                                     GameObject &target,
                                     const scene::AABB &collisionAABB) {
    const auto &center = collisionAABB.center();
    std::println(std::clog,
                 "ModelParticle collided with {} at AABB center ({}, {}, {})",
                 target.getName(), center.x, center.y, center.z);
    particle.velocity = -particle.velocity * 0.8f;
    particle.life *= 1.5;
    if (particle.velocity.y <= 0.001f) {
      particle.life = 0.0f;
    }
  }

private:
  ModelParticleSystem *particleSystem;
  std::vector<GameObject *> collisionTargets;
  float collisionMargin;

  /// Check AABB-AABB collision between ModelParticle and GameObject
  /// @param particle ModelParticle to check
  /// @param target GameObject to check
  /// @param particleSystemModel Parent transformation matrix
  /// @param collisionAABB Output: overlapping AABB region
  /// @return True if collision detected
  bool checkAABBCollision(const ModelParticle &particle,
                          const GameObject &target,
                          const glm::mat4 &particleSystemModel,
                          scene::AABB &collisionAABB) const;
};

inline void ModelParticleAABBCollisionBehaviour::update(Particle &particle,
                                                        float deltaTime,
                                                        const glm::mat4 &model,
                                                        SimulationSpace space) {
  BaseBehaviour::update(particle, deltaTime, model, space);

  // Cast to ModelParticle (this behaviour only works with ModelParticle)
  auto &modelParticle = static_cast<ModelParticle &>(particle);

  // Skip if particle is already dead
  if (modelParticle.life <= 0.0f) {
    return;
  }

  // Check collision with all targets
  for (auto target : collisionTargets) {
    if (!target) {
      continue;
    }

    scene::AABB collisionAABB;
    if (checkAABBCollision(modelParticle, *target, model, collisionAABB)) {
      onGameObjectCollision(modelParticle, *target, collisionAABB);
      break; // Stop after first collision
    }
  }
}

inline bool ModelParticleAABBCollisionBehaviour::checkAABBCollision(
    const ModelParticle &particle, const GameObject &target,
    const glm::mat4 &particleSystemModel, scene::AABB &collisionAABB) const {
  if (!particleSystem) {
    return false;
  }

  // 1. Get particle's world AABB
  // Transform from particle local space to particle system space to world space
  glm::mat4 particleTransform =
      particleSystemModel * particle.getTransformMatrix();
  scene::AABB particleLocalAABB = particleSystem->getParticleLocalAABB();
  scene::AABB particleWorldAABB =
      particleLocalAABB.transform(particleTransform);

  // 2. Get target's world AABB
  scene::AABB targetWorldAABB = target.getWorldAABB();

  // 3. Check AABB overlap with margin
  if (particleWorldAABB.overlaps(targetWorldAABB, collisionMargin)) {
    // Calculate overlapping region (for debugging)
    glm::vec3 overlapMin = glm::max(particleWorldAABB.min, targetWorldAABB.min);
    glm::vec3 overlapMax = glm::min(particleWorldAABB.max, targetWorldAABB.max);
    collisionAABB = scene::AABB{overlapMin, overlapMax};
    return true;
  }

  return false;
}
} // namespace graphics::particles
