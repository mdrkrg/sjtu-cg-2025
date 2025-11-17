#pragma once

#include "particle_emitter.hpp"
#include "particle.hpp"
#include "movement.hpp"
#include <shader.h>
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <queue>

class ParticleSystem {
public:
  ParticleSystem(std::shared_ptr<ParticleEmitter> emitter);
  ~ParticleSystem();

  /// Update all particles
  void update(float deltaTime, const glm::mat4 &model);

  /// Render all particles
  void render(Shader &shader, const glm::mat4 &model, const glm::mat4 &view,
              const glm::mat4 &projection);

  /// Emit particles dynamically
  void emitParticle();

  /// Emit a burst amout of particles
  void emitBurst(int count);

  /// Set maximum number of particles
  void setMaxParticles(size_t max) { maxParticles = max; }

  /// Get current particle count
  size_t getActiveParticleCount() const { return activeParticles.size(); }

  /// Reset the system
  void reset();

  /// Initialize OpenGL resources
  void init();

  void processKeyboard(Movement direction, float deltaTime);

  inline void toggle(bool toggled) { this->toggled = toggled; }

private:
  std::shared_ptr<ParticleEmitter> emitter;
  std::vector<Particle> activeParticles;
  std::queue<Particle> particlePool; // For reuse
  size_t maxParticles = 1000;

  bool toggled = false;

  // OpenGL resources
  unsigned int VAO = 0, VBO = 0;
  unsigned int particleTexture = 0;

  /// Setup OpenGL buffers
  void setupBuffers();
  /// Setup OpenGL textures
  void loadTexture();

  /// Cleanup OpenGL resources
  void cleanup();

  /// Update buffer data
  void updateBuffers();
};
