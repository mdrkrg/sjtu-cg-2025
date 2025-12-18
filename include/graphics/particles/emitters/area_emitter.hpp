#pragma once

#include "base_emitter.hpp"
#include <glm/glm.hpp>

/// Emitter that emits particle within a square area.
class AreaEmitter : public BaseEmitter {
public:
  AreaEmitter(std::shared_ptr<ParticleBehaviour> behaviour,
              const glm::vec3 &center = glm::vec3(0.0f),
              const glm::vec3 &size = glm::vec3(1.0f))
      : BaseEmitter(behaviour), center(center), size(size) {}
  virtual ~AreaEmitter() = default;

  /// Emit a particle from a random point within an area
  inline virtual void emit(Particle &particle) override {
    BaseEmitter::emit(particle);

    // Generate random position within the area
    float x = center.x + (uniformDist(gen) - 0.5f) * size.x;
    float y = center.y + (uniformDist(gen) - 0.5f) * size.y;
    float z = center.z + (uniformDist(gen) - 0.5f) * size.z;

    particle.position = glm::vec3(x, y, z);
  }

  void handleMovement(Movement movement, float deltaTime) override {
    // Move along X or Z
    static glm::vec3 front{0.0, 0.0, -1.0};
    static glm::vec3 right{1.0, 0.0, 0.0};

    float velocity = movementSpeed * deltaTime;

    if (movement == FORWARD)
      center += front * velocity;
    if (movement == BACKWARD)
      center -= front * velocity;
    if (movement == LEFT)
      center -= right * velocity;
    if (movement == RIGHT)
      center += right * velocity;
  }

  /// Set area parameters
  void setArea(const glm::vec3 &center, const glm::vec3 &size) {
    this->center = center;
    this->size = size;
  }

  /// Set area center
  void setCenter(const glm::vec3 &center) { this->center = center; }

  /// Set area size
  void setSize(const glm::vec3 &size) { this->size = size; }

private:
  glm::vec3 center;
  glm::vec3 size;
};
