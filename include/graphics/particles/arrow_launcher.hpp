#pragma once

#include "graphics/particles/updaters/arrow_collision_updater.hpp"
#include "model_particle_system.hpp"
#include "emitter_group.hpp"
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
  /// @param emissionPolicy Emission policy for multiple emitters
  explicit ArrowLauncher(
      std::shared_ptr<GameManager> gameManager,
      EmissionPolicy emissionPolicy = EmissionPolicy::RoundRobin);

  /// Initialize arrow launcher with resources
  /// @param arrowModelWithMaterials Shared arrow model with materials
  /// @param arrowShader Shared instanced rendering shader
  /// @param maxArrows Maximum number of arrows per system
  void init(ModelWithMaterials arrowModelWithMaterials,
            std::shared_ptr<Shader> arrowShader, size_t maxArrows = 1000);

  /// Add an emitter at specified position and direction
  /// @param position World position of emitter
  /// @param direction Emission direction (normalized)
  /// @param speed Arrow speed (units per second)
  /// @param spreadAngle Spread angle in degrees
  void addEmitter(const glm::vec3 &position,
                  const glm::vec3 &direction = glm::vec3{0.0f, 0.0f, -1.0f},
                  float speed = 10.0f, float spreadAngle = 5.0f);

  /// Remove an emitter by index
  /// @param index Index of emitter to remove
  void removeEmitter(size_t index);

  /// Clear all emitters
  void clearEmitters();

  /// Get number of emitters
  /// @return Emitter count
  size_t getEmitterCount() const;

  /// Get emitter group
  /// @return Pointer to emitter group
  std::shared_ptr<EmitterGroup<ModelParticleSystem>> getEmitterGroup() const {
    return emitterGroup;
  }

  /// Fire a volley of arrows from all emitters
  /// @param arrowsPerEmitter Arrows to fire from each emitter
  void launch(size_t arrowsPerEmitter = 5);

  /// Start continuous firing from all emitters
  /// @param arrowsPerSecond Total arrows per second across all emitters
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

  /// Render all arrow systems
  /// @param view View matrix
  /// @param projection Projection matrix
  void render(const glm::mat4 &view, const glm::mat4 &projection);

  /// Get total active arrow count across all systems
  /// @return Total active arrows
  size_t getTotalActiveArrows() const;

  /// Check if arrow launcher is initialized
  /// @return True if initialized
  bool isInitialized() const { return initialized; }

  /// Set emission policy
  /// @param policy New emission policy
  void setEmissionPolicy(EmissionPolicy policy);

private:
  std::shared_ptr<GameManager> gameManager;
  std::shared_ptr<EmitterGroup<ModelParticleSystem>> emitterGroup;
  std::shared_ptr<ArrowCollisionUpdater> collisionUpdater;
  ModelWithMaterials arrowModelWithMaterials;
  std::shared_ptr<Shader> arrowShader;
  size_t maxArrowsPerSystem{1000};
  bool initialized{false};

  /// Create a new arrow system for an emitter
  /// @param position Emitter position
  /// @param direction Emitter direction
  /// @param speed Arrow speed
  /// @param spreadAngle Spread angle
  /// @param gravity Gravity applied to the arrow
  /// @param drag Drag applied to the arrow
  /// @param lifetime Initial lifetime of the arrow
  /// @return New arrow particle system
  std::shared_ptr<ModelParticleSystem>
  createArrowSystem(const glm::vec3 &position, const glm::vec3 &direction,
                    float speed, float spreadAngle, float gravity = 9.81,
                    float drag = 0.01, float lifetime = 10.0);
};

} // namespace graphics::particles
