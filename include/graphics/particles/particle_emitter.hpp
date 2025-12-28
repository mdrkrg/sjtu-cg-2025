#pragma once

#include "scene/game_object.hpp"
#include "particle_behaviour.hpp"
#include "particle.hpp"
#include <glm/glm.hpp>
#include <memory>
#include <random>

template <typename ParticleType> class ParticleSystem;

class ParticleEmitter {
public:
  ParticleEmitter(std::shared_ptr<ParticleBehaviour> behaviour,
                  float emissionRate = 0.0f, const GameObject *parent = nullptr,
                  SimulationSpace space = SimulationSpace::WORLD)
      : behaviour(behaviour), emissionRate(emissionRate), parent(parent),
        simulationSpace(space) {}
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

  /// Set associated behaviour
  void setBehaviour(std::shared_ptr<ParticleBehaviour> newBehaviour) {
    behaviour = newBehaviour;
  }

  /// Update emission timing and return number of particles to emit
  size_t updateEmission(float deltaTime) {
    size_t totalEmitCount = 0;
    // Handle continuous emission
    if (emissionRate > 0.0f) {
      totalEmitCount += handleEmission(deltaTime);
    }

    // Handle burst emission
    if (burstCount > 0) {
      if (burstInterval > 0.0f) {
        // Scheduled burst: wait for interval
        timeSinceLastBurst += deltaTime;
        if (timeSinceLastBurst >= burstInterval) {
          totalEmitCount += burstCount;
          timeSinceLastBurst = 0.0f;
          burstCount = 0; // Clear burst after emitting
        }
      } else if (burstInterval == 0.0f) {
        // Immediate burst: emit now
        totalEmitCount += burstCount;
        burstCount = 0; // Clear burst after emitting
      }
      // If burstInterval < 0.0f, do nothing (invalid interval)
    }

    return totalEmitCount;
  }

  /// Reset emission timing
  void resetEmission() {
    timeSinceLastEmission = 0.0f;
    timeSinceLastBurst = 0.0f;
  }

protected:
  std::shared_ptr<ParticleBehaviour> behaviour;
  float emissionRate = 10.0f; // particles per second

  int burstCount = 0;
  float burstInterval = 0.0f;

  float timeSinceLastEmission = 0.0f;
  float timeSinceLastBurst = 0.0f;

  static std::random_device rd;
  static std::mt19937 gen;

  template <typename ParticleType> friend class ParticleSystem;

  const GameObject *parent = nullptr;
  SimulationSpace simulationSpace = SimulationSpace::WORLD;

  /// Calculate particle position by simulation space
  glm::vec3
  calculatePositionBySimulationSpace(const glm::vec3 &localPos) const {
    if (simulationSpace == SimulationSpace::WORLD and parent) {
      // Transform world space using parent matrix
      const auto parentMatrix = parent->getModelMatrix();
      return parentMatrix * glm::vec4(localPos, 1.0f);
    } else {
      // LOCAL or CUSTOM space, or no parent
      // Use local coordinates
      return localPos;
    }
  }

  glm::vec3
  calculateVelocityBySimulationSpace(const glm::vec3 &localVel) const {
    if (simulationSpace == SimulationSpace::WORLD && parent) {
      const auto parentMatrix = parent->getModelMatrix();
      // Use 0 for the w component
      // w = 1 for points (translation)
      // w = 0 for vectors (ignore translation, apply rotation and scale)
      return glm::vec3(parentMatrix * glm::vec4(localVel, 0.0f));
    } else {
      // LOCAL or CUSTOM space
      // The renderer will rotate it later
      return localVel;
    }
  }

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
