#pragma once

#include "particles/particle_emitter.hpp"
#include <glm/glm.hpp>
#include <random>

constexpr float PARTICLE_SPEED = 28.0f;

class BaseEmitter : public ParticleEmitter {
public:
  BaseEmitter(std::shared_ptr<ParticleBehaviour> behaviour)
      : ParticleEmitter(behaviour) {}
  virtual ~BaseEmitter() = default;

  /// Emit a particle with emitter-specific positioning
  virtual void emit(Particle &particle) override {
    ParticleEmitter::emit(particle);

    // Set initial position (can be overridden)
    particle.position = position;
  }

  const glm::vec3 &getPosition() const { return position; }
  void setPosition(const glm::vec3 &position) { this->position = position; }

  void handleMovement(Movement movement, float deltaTime) override {
    // Move along X or Z
    static glm::vec3 front{0.0, 0.0, -1.0};
    static glm::vec3 right{1.0, 0.0, 0.0};

    float velocity = movementSpeed * deltaTime;

    if (movement == FORWARD)
      position += front * velocity;
    if (movement == BACKWARD)
      position -= front * velocity;
    if (movement == LEFT)
      position -= right * velocity;
    if (movement == RIGHT)
      position += right * velocity;
  }

protected:
  // Position and direction for emission
  glm::vec3 position = glm::vec3(0.0f);
  glm::vec3 direction = glm::vec3(0.0f, 1.0f, 0.0f);

  float movementSpeed = PARTICLE_SPEED;

  static std::random_device rd;
  static std::mt19937 gen;
  static std::uniform_real_distribution<float> uniformDist;
};

inline std::random_device BaseEmitter::rd{};
inline std::mt19937 BaseEmitter::gen{rd()};
inline std::uniform_real_distribution<float> BaseEmitter::uniformDist{0.0f,
                                                                      1.0f};
