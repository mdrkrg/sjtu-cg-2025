#pragma once

#include "graphics/particles/updaters/arrow_collision_updater.hpp"
#include "model_particle_system.hpp"
#include "emitters/arrow_emitter.hpp"
#include "scene/game_manager.hpp"
#include <memory>
#include <vector>

namespace graphics::particles {

/// High-level coordinator for arrow launcher system
/// Manages multiple emitters, collision targets, and scene integration
class ArrowLauncher {
public:
  /// Constructor
  /// @param gameManager GameManager for scene integration
  explicit ArrowLauncher(std::shared_ptr<GameManager> gameManager);

  /// Initialize arrow launcher with resources
  /// @param arrowModelWithMaterials Shared arrow model with materials
  /// @param arrowShader Shared instanced rendering shader
  /// @param maxArrows Maximum number of arrows in system
  void init(ModelWithMaterials arrowModelWithMaterials,
            std::shared_ptr<Shader> arrowShader, size_t maxArrows = 1000);

  /// Set emitter position and direction
  /// @param position World position of emitter
  /// @param direction Emission direction (normalized)
  /// @param speed Arrow speed (units per second)
  /// @param spreadAngle Spread angle in degrees
  void setEmitter(const glm::vec3 &position,
                  const glm::vec3 &direction = glm::vec3{0.0f, 0.0f, -1.0f},
                  float speed = 10.0f, float spreadAngle = 5.0f);

  /// Get the emitter
  /// @return Pointer to emitter
  std::shared_ptr<ArrowEmitter> getEmitter() const { return emitter; }

  /// Fire a volley of arrows from all emitters
  /// @param arrowsPerEmitter Arrows to fire from each emitter
  void launch(size_t arrowsPerEmitter = 5);

  /// Start continuous firing from all emitters
  /// @param arrowsPerSecond Arrows per second per emitter
  void startContinuousFire(float arrowsPerSecond);

  /// Stop continuous firing
  void stopContinuousFire();

  /// Add collision target for all arrows
  /// @param target GameObject to check for collision
  void addCollisionTarget(GameObject *target);

  /// Remove collision target
  /// @param target GameObject to remove from collision checks
  void removeCollisionTarget(GameObject *target);

  /// Clear all collision targets
  void clearCollisionTargets();

  /// Update arrow launcher (call each frame)
  /// @param deltaTime Time since last update
  void update(float deltaTime);

  /// Get the arrow particle system
  /// @return Pointer to ModelParticleSystem
  std::shared_ptr<ModelParticleSystem> getArrowSystem() const {
    return arrowSystem;
  }

  /// Check if arrow launcher is initialized
  /// @return True if initialized
  bool isInitialized() const { return initialized; }

private:
  std::shared_ptr<GameManager> gameManager;
  std::shared_ptr<ModelParticleSystem> arrowSystem;
  std::shared_ptr<ArrowEmitter> emitter;
  // std::shared_ptr<ArrowCollisionBehaviour> collisionBehaviour;
  std::shared_ptr<ArrowCollisionUpdater> collisionUpdater;
  std::vector<GameObject *> collisionTargets;
  bool initialized{false};
};

} // namespace graphics::particles
