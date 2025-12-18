#pragma once

#include "core/movement.hpp"
#include "particle_behaviour.hpp"
#include "particle.hpp"
#include <glm/glm.hpp>
#include <memory>
#include <random>

class ParticleEmitter {
public:
  ParticleEmitter(std::shared_ptr<ParticleBehaviour> behaviour,
                  float emissionRate = 0.0f)
      : behaviour(behaviour), emissionRate(emissionRate) {}
  virtual ~ParticleEmitter() = default;

  /// Emit a single particle
  inline virtual void emit(Particle &particle) {
    if (behaviour) {
      behaviour->initialize(particle);
    }
  }

  /// Set emission rate (particles per second)
  void setEmissionRate(float rate) { emissionRate = rate; }

  /// Set burst parameters
  void setBurst(size_t particleCount, float intervalSeconds) {
    burstCount = particleCount;
    burstInterval = intervalSeconds;
  }

  /// Get associated behaviour
  std::shared_ptr<ParticleBehaviour> getBehaviour() const { return behaviour; }

  /// Update emission timing and return number of particles to emit
  size_t updateEmission(float deltaTime) {
    size_t totalEmitCount = 0;
    // Handle continuous emission
    if (emissionRate > 0.0f) {
      totalEmitCount += handleEmission(deltaTime);
    }

    // Handle burst emission
    if (burstCount > 0 && burstInterval > 0.0f) {
      timeSinceLastBurst += deltaTime;
      if (timeSinceLastBurst >= burstInterval) {
        totalEmitCount += burstCount;
        timeSinceLastBurst = 0.0f;
      }
    }

    return totalEmitCount;
  }

  /// Reset emission timing
  void resetEmission() {
    timeSinceLastEmission = 0.0f;
    timeSinceLastBurst = 0.0f;
  }

  /// Process emitter movement. Subclasses can override.
  virtual void handleMovement(Movement movement, float deltaTime) {}

protected:
  std::shared_ptr<ParticleBehaviour> behaviour;
  float emissionRate = 10.0f; // particles per second

  int burstCount = 0;
  float burstInterval = 0.0f;

  float timeSinceLastEmission = 0.0f;
  float timeSinceLastBurst = 0.0f;

  static std::random_device rd;
  static std::mt19937 gen;

private:
  size_t handleEmission(float deltaTime) {
    timeSinceLastEmission += deltaTime;
    const auto particlesToEmit = emissionRate * timeSinceLastEmission;
    const auto emitCount = static_cast<size_t>(particlesToEmit);
    // Update time
    timeSinceLastEmission -= emitCount / emissionRate;
    return emitCount;
  }
};

inline std::random_device ParticleEmitter::rd{};
inline std::mt19937 ParticleEmitter::gen{rd()};
