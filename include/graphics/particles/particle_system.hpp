#pragma once

#include "base/death_check.hpp"
#include "base/emitter.hpp"
#include "base/updater.hpp"
#include "graphics/particles/death_checks/timeout_death_check.hpp"
#include "particle.hpp"
#include "scene/game_object.hpp"
#include <graphics/shader.h>
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <queue>

namespace graphics::particles {

template <typename ParticleType = Particle> class ParticleSystem {
public:
  ParticleSystem(std::shared_ptr<Shader> shader,
                 std::shared_ptr<Emitter> emitter,
                 SimulationSpace space = SimulationSpace::WORLD,
                 GameObject *customTransform = nullptr)
      : shader{shader}, emitter(emitter), simulationSpace(space),
        customSimulationTransform(customTransform) {
    // Set emitter's simulation space
    if (emitter) {
      emitter->simulationSpace = space;
    }
  }

  virtual ~ParticleSystem() { cleanup(); }

  /// Update all particles
  virtual void update(float deltaTime) {
    const auto &model = getModelMatrix();
    // Update existing particles
    auto it = activeParticles.begin();
    while (it != activeParticles.end()) {
      ParticleType &p = *it;

      // Update life
      // WARN: Life should not update here?
      // TODO: Move life update to the actual base behaviour?
      p.life -= deltaTime;

      // Check if particle should die
      const std::optional<DeathReason> deathReason = [this, &p] {
        for (const auto &checker : deathChecks) {
          if (checker->shouldCheck(p)) {
            if (const auto reason = checker->check(p); reason.has_value()) {
              // Notify death event
              checker->onDeath(p, reason.value());
              return reason; // Only check once
            }
          }
        }
        return std::optional<DeathReason>(std::nullopt);
      }();

      if (deathReason.has_value()) {
        // Notify all updaters of death
        for (const auto &updater : updaters) {
          updater->onDeath(p, deathReason.value());
        }
        // Add to pool for reuse
        particlePool.push(p);
        it = activeParticles.erase(it);
      } else {
        // Update particle behaviour
        for (const auto &updater : updaters) {
          updater->update(p, deltaTime, model, simulationSpace);
        }

        // Update position
        p.position += p.velocity * deltaTime;
        ++it;
      }
    }

    // Automatic emission
    if (emitter and toggled) {
      int emitCount = emitter->updateEmission(deltaTime);
      for (int i = 0; i < emitCount && activeParticles.size() < maxParticles;
           ++i) {
        emitParticle();
      }
    }

    // Update buffer data
    updateBuffers();
  }

  /// Render all particles
  virtual void render(const glm::mat4 &view, const glm::mat4 &projection) {
    const auto &model = getModelMatrix();
    if (activeParticles.empty()) {
      return;
    }

    shader->use();

    { // Uniforms
      shader->setMat4("view", view);
      shader->setMat4("projection", projection);
      shader->setMat4("model", model);
    }

    { // Bind texture
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, particleTexture);
      shader->setInt("particleTexture", 0);
    }

    // Update buffer data before rendering
    updateBuffers();

    // Render particles
    glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, activeParticles.size());
    glBindVertexArray(0);
  }

  /// Emit particles dynamically
  virtual void emitParticle() {
    if (not toggled) {
      return;
    }
    if (activeParticles.size() >= maxParticles) {
      // Maximum particles exceeded
      return;
    }

    auto p = [this] {
      if (particlePool.empty()) {
        // Create new particle if empty
        return ParticleType{};
      }
      // Reuse
      const auto p = particlePool.front();
      particlePool.pop();
      return p;
    }();

    // Emit the particle
    if (emitter) {
      emitter->emit(p);
    }

    activeParticles.push_back(p);
  }

  /// Emit a burst amout of particles
  virtual void emitBurst(int count) {
    if (not toggled) {
      return;
    }

    for (int i = 0; i < count && activeParticles.size() < maxParticles; ++i) {
      emitParticle();
    }
  }

  /// Set maximum number of particles
  void setMaxParticles(size_t max) { maxParticles = max; }

  /// Get current particle count
  size_t getActiveParticleCount() const { return activeParticles.size(); }

  /// Reset the system
  virtual void reset() {
    // Move all active particles to the pool
    for (const auto &p : activeParticles) {
      particlePool.push(p);
    }
    activeParticles.clear();
  }

  /// Initialize OpenGL resources
  virtual void init() {
    setupBuffers();
    loadTexture();
  }

  void toggle(bool toggled) { this->toggled = toggled; }

  void setShader(std::shared_ptr<Shader> shader) { this->shader = shader; }

  void addUpdater(std::shared_ptr<Updater> updater) {
    updaters.push_back(updater);
  }

  /// Add a death check to the system
  /// @param deathCheck Death check to add
  void addDeathCheck(std::shared_ptr<DeathCheck> deathCheck) {
    deathChecks.push_back(deathCheck);
  }

  /// Remove all death checks
  void clearDeathChecks() { deathChecks.clear(); }

  static std::shared_ptr<DeathCheck> defaultDeathCheck() {
    return std::make_shared<TimeoutDeathCheck>();
  }

protected:
  std::shared_ptr<Emitter> emitter;
  std::vector<ParticleType> activeParticles;
  std::queue<ParticleType> particlePool; // For reuse
  size_t maxParticles = 1000;

  std::vector<std::shared_ptr<Updater>> updaters;
  std::vector<std::shared_ptr<DeathCheck>> deathChecks;

  std::shared_ptr<Shader> shader;

  bool toggled = false;

  // Simulation space configuration
  SimulationSpace simulationSpace;
  GameObject *customSimulationTransform;

  /// Get model matrix based on simulation space
  glm::mat4 getModelMatrix() const {
    switch (simulationSpace) {
    case SimulationSpace::WORLD:
      // Identity
      return glm::mat4{1.0f};
    case SimulationSpace::LOCAL:
      // For LOCAL space, use emitter's parent
      if (emitter && emitter->parent) {
        return emitter->parent->getModelMatrix();
      }
      // Fallback to identity if no parent
      return glm::mat4{1.0f};
    case SimulationSpace::CUSTOM:
      // For CUSTOM space, use the custom transform
      if (customSimulationTransform) {
        return customSimulationTransform->getModelMatrix();
      }
      // Fallback to identity if no custom transform
      return glm::mat4(1.0f);
    default:
      return glm::mat4(1.0f);
    }
  }

  // OpenGL resources
  unsigned int VAO = 0, VBO = 0;
  unsigned int particleTexture = 0;

  /// Setup OpenGL buffers
  virtual void setupBuffers() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, maxParticles * sizeof(ParticleType), nullptr,
                 GL_DYNAMIC_DRAW);

    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleType),
                          (void *)0);

    // Color attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleType),
                          (void *)offsetof(ParticleType, color));

    // Size attribute
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleType),
                          (void *)offsetof(ParticleType, size));

    glBindVertexArray(0);
  }

  /// Setup OpenGL textures
  virtual void loadTexture() {
    // Create a simple white texture for particles
    glGenTextures(1, &particleTexture);
    glBindTexture(GL_TEXTURE_2D, particleTexture);

    // Create a simple white texture
    unsigned char whitePixel[3] = {255, 255, 255};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE,
                 whitePixel);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }

  /// Cleanup OpenGL resources
  virtual void cleanup() {
    if (VAO != 0) {
      glDeleteVertexArrays(1, &VAO);
      VAO = 0;
    }
    if (VBO != 0) {
      glDeleteBuffers(1, &VBO);
      VBO = 0;
    }
    if (particleTexture != 0) {
      glDeleteTextures(1, &particleTexture);
      particleTexture = 0;
    }

    // Clear particles
    activeParticles.clear();
    std::queue<ParticleType> empty;
    std::swap(particlePool, empty);
  }

  /// Update buffer data
  virtual void updateBuffers() {
    if (activeParticles.empty())
      return;

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    activeParticles.size() * sizeof(ParticleType),
                    activeParticles.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }
};
} // namespace graphics::particles
