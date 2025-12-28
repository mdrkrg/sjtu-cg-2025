#pragma once

#include "particle.hpp"
#include <glm/glm.hpp>

/// Interface of the behaviour of particles
class ParticleBehaviour {
public:
  virtual ~ParticleBehaviour() = default;

  /// Initialize a particle with behaviour-specific properties
  virtual void initialize(Particle &particle) = 0;

  /// Update particle state based on behaviour
  virtual void update(Particle &particle, float deltaTime,
                      const glm::mat4 &model, SimulationSpace space) = 0;

  /// Check if particle is still alive
  virtual bool isAlive(const Particle &particle, const glm::mat4 &model,
                       SimulationSpace space) const = 0;

  /// Handle particle death events
  virtual void onDeath(Particle &particle) { (void)particle; }

  /// Handle particle respawn events
  virtual void onRespawn(Particle &particle) { (void)particle; }
};
