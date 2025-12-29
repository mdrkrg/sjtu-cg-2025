#pragma once

#include "base_emitter.hpp"
#include "math/random.hpp"
#include <glm/glm.hpp>

namespace graphics::particles {

/// Emitter that emits particle within a square area.
class AreaEmitter : public BaseEmitter {
public:
  AreaEmitter(std::shared_ptr<Initializer> initializer = nullptr,
              const glm::vec3 &position = glm::vec3(0.0f),
              const glm::vec3 &size = glm::vec3(1.0f),
              const GameObject *parent = nullptr,
              SimulationSpace space = SimulationSpace::WORLD)
      : BaseEmitter(initializer, parent, space), size(size) {
    this->position = position;
  }
  virtual ~AreaEmitter() = default;

  /// Emit a particle from a random point within an area
  inline virtual void emit(Particle &particle) override {
    BaseEmitter::emit(particle);

    // Generate random position within the area (local coordinates)
    const auto &position = getEmissionOrigin();
    float x = position.x + (math::uniformDist() - 0.5f) * size.x;
    float y = position.y + (math::uniformDist() - 0.5f) * size.y;
    float z = position.z + (math::uniformDist() - 0.5f) * size.z;

    glm::vec3 localPos = glm::vec3(x, y, z);

    // Transform based on simulation space
    particle.position = calculatePositionBySimulationSpace(localPos);
  }

  /// Set area parameters
  void setArea(const glm::vec3 &position, const glm::vec3 &size) {
    this->position = position;
    this->size = size;
  }

  /// Set area position
  void setPosition(const glm::vec3 &position) { this->position = position; }

  /// Set area size
  void setSize(const glm::vec3 &size) { this->size = size; }

private:
  glm::vec3 size;
};
} // namespace graphics::particles
