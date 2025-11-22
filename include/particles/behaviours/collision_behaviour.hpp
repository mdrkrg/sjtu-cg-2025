#pragma once

#include "particles/behaviours/base_behaviour.hpp"
#include <glm/glm.hpp>
#include <memory>
#include <optional>
#include "terrain_mesh.hpp"

/// Behaviour that triggers a callback when on collision with a terrain.
class CollisionBehaviour : public BaseBehaviour {
public:
  inline CollisionBehaviour(std::shared_ptr<TerrainMesh> terrain,
                            const glm::mat4 &terrainModel)
      : terrain(terrain), terrainModel(terrainModel) {}
  virtual ~CollisionBehaviour() = default;

  /// Update particle with collision detection
  inline virtual void update(Particle &particle, float deltaTime,
                             const glm::mat4 &model) override {
    BaseBehaviour::update(particle, deltaTime, model);

    // Check for collision with terrain
    if (const auto heightResult = getCollisionHeight(particle, model);
        heightResult) {
      onCollision(particle, heightResult.value());
    }
  }

  inline virtual bool isAlive(const Particle &particle,
                              const glm::mat4 &model) const override {
    if (!BaseBehaviour::isAlive(particle, model)) {
      return false;
    }

    // Fallen too far below
    return particle.position.y > -10.0f;
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

/// CollisionBehaviour that covers the terrain with particles on collision.
class CoverBehaviour : public CollisionBehaviour {
public:
  inline CoverBehaviour(std::shared_ptr<TerrainMesh> terrain,
                        const glm::mat4 &terrainModel)
      : CollisionBehaviour(terrain, terrainModel) {}

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

/// CollisionBehaviour that eliminate the particle on collision with the
/// terrain.
class DissolveBehaviour : public CollisionBehaviour {
public:
  inline DissolveBehaviour(std::shared_ptr<TerrainMesh> terrain,
                           const glm::mat4 &terrainModel)
      : CollisionBehaviour(terrain, terrainModel) {}

protected:
  inline void onCollision(Particle &particle, float) const {
    // Eliminate if touching terrain
    particle.life = 0.0f;
  }
};
