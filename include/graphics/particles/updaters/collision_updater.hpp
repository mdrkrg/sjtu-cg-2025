#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <optional>
#include "graphics/particles/updaters/base_updater.hpp"
#include "graphics/terrain_mesh.hpp"

namespace graphics::particles {

/// Updater that triggers a callback when on collision with a terrain.
class CollisionUpdater : public BaseUpdater {
public:
  inline CollisionUpdater(std::shared_ptr<TerrainMesh> terrain,
                          const glm::mat4 &terrainModel)
      : terrain(terrain), terrainModel(terrainModel) {}
  virtual ~CollisionUpdater() = default;

  /// Update particle with collision detection
  inline virtual void update(Particle &particle, float deltaTime,
                             const glm::mat4 &model,
                             SimulationSpace space) override {
    BaseUpdater::update(particle, deltaTime, model, space);

    // Check for collision with terrain
    if (const auto heightResult = getCollisionHeight(particle, model);
        heightResult) {
      onCollision(particle, heightResult.value());
    }
  }

protected:
  /// Child classes should implement
  virtual void onCollision(Particle &, float) const {
    // Default no behaviour
  };

private:
  std::shared_ptr<TerrainMesh> terrain;
  glm::mat4 terrainModel;

  /// Get height on collision, otherwise returns nullopt
  inline std::optional<float> getCollisionHeight(const Particle &particle,
                                                 const glm::mat4 &model) const {
    if (!terrain) {
      return std::nullopt;
    }

    // Transform to world space
    const auto worldPos = model * glm::vec4(particle.position, 1.0f);

    // Transform to terrain local space
    const auto invTerrainModel = glm::inverse(terrainModel);
    const auto localPos = invTerrainModel * worldPos;

    const auto terrainHeight = terrain->getHeightAt(localPos.x, localPos.z);

    // Check if particle has hit or gone below the terrain, or out of bound
    if (std::isnan(terrainHeight) or localPos.y <= terrainHeight) {
      return terrainHeight;
    }
    return std::nullopt;
  }
};

/// CollisionUpdater that covers the terrain with particles on collision.
class CoverUpdater : public CollisionUpdater {
public:
  inline CoverUpdater(std::shared_ptr<TerrainMesh> terrain,
                      const glm::mat4 &terrainModel)
      : CollisionUpdater(terrain, terrainModel) {}

protected:
  inline void onCollision(Particle &particle, float height) const {
    if (std::isnan(height)) {
      particle.life = 0.0f;
      return;
    }
    // Stop movement when hitting terrain
    particle.velocity = glm::vec3(0.0f);
    particle.life += 5.0f;
  }
};

/// CollisionUpdater that eliminate the particle on collision with the
/// terrain.
class DissolveUpdater : public CollisionUpdater {
public:
  inline DissolveUpdater(std::shared_ptr<TerrainMesh> terrain,
                         const glm::mat4 &terrainModel)
      : CollisionUpdater(terrain, terrainModel) {}

protected:
  inline void onCollision(Particle &particle, float) const {
    // Eliminate if touching terrain
    particle.life = 0.0f;
  }
};
} // namespace graphics::particles
