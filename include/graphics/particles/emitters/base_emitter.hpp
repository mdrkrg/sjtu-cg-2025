#pragma once

#include "graphics/particles/particle_emitter.hpp"
#include <glm/glm.hpp>
#include <random>

constexpr float PARTICLE_SPEED = 28.0f;

class BaseEmitter : public ParticleEmitter {
public:
  BaseEmitter(std::shared_ptr<ParticleBehaviour> behaviour,
              const GameObject *parent = nullptr,
              SimulationSpace space = SimulationSpace::WORLD)
      : ParticleEmitter(behaviour, 0.0f, parent, space) {}
  virtual ~BaseEmitter() = default;

  /// Emit a particle with emitter-specific positioning
  virtual void emit(Particle &particle) override {
    ParticleEmitter::emit(particle);

    // Get emission origin (local coordinates)
    glm::vec3 localPos = getEmissionOrigin();

    // Transform based on simulation space
    particle.position = calculatePositionBySimulationSpace(localPos);
  }

  /// Get emission offset relative to the origin
  glm::vec3 getEmissionOrigin() const { return position; }

protected:
  // Position and direction for emission
  glm::vec3 position = glm::vec3(0.0f);
  glm::vec3 direction = glm::vec3(0.0f, 1.0f, 0.0f);

  static std::random_device rd;
  static std::mt19937 gen;
  static std::uniform_real_distribution<float> uniformDist;
};

inline std::random_device BaseEmitter::rd{};
inline std::mt19937 BaseEmitter::gen{rd()};
inline std::uniform_real_distribution<float> BaseEmitter::uniformDist{0.0f,
                                                                      1.0f};
