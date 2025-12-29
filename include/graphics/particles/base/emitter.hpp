#pragma once

#include "scene/game_object.hpp"
#include "graphics/particles/particle.hpp"
#include "initializer.hpp"
#include <glm/glm.hpp>
#include <memory>

namespace graphics::particles {

class Emitter {
public:
  Emitter(std::shared_ptr<Initializer> initializer, float emissionRate = 0.0f,
          const GameObject *parent = nullptr,
          SimulationSpace space = SimulationSpace::WORLD)
      : initializer{initializer}, emissionRate{emissionRate}, parent{parent},
        simulationSpace{space} {}

  virtual ~Emitter() = default;

  /// Emit a single particle
  inline virtual void emit(Particle &particle) {
    if (initializer) {
      initializer->initialize(particle);
    }
  }

  /// Set emission rate (particles per second)
  void setEmissionRate(float rate) { emissionRate = rate; }

  /// Set burst parameters
  void setBurst(size_t particleCount, float intervalSeconds) {
    burstCount = particleCount;
    burstInterval = intervalSeconds;
  }

  /// Get associated initializer
  std::shared_ptr<Initializer> getInitializer() const { return initializer; }

  /// Set associated initializer
  void setInitializer(std::shared_ptr<Initializer> newInitializer) {
    initializer = newInitializer;
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
  std::shared_ptr<Initializer> initializer;
  float emissionRate = 10.0f; // particles per second

  int burstCount = 0;
  float burstInterval = 0.0f;

  float timeSinceLastEmission = 0.0f;
  float timeSinceLastBurst = 0.0f;

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
} // namespace graphics::particles
