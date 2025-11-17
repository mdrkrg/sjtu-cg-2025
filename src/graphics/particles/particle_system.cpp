#include "particles/particle_system.hpp"
#include <glad/glad.h>
#include <algorithm>

ParticleSystem::ParticleSystem(std::shared_ptr<ParticleEmitter> emitter)
    : emitter(emitter) {}

ParticleSystem::~ParticleSystem() { cleanup(); }

void ParticleSystem::init() {
  setupBuffers();
  loadTexture();
}

void ParticleSystem::update(float deltaTime, const glm::mat4 &model) {
  // Update existing particles
  auto it = activeParticles.begin();
  while (it != activeParticles.end()) {
    Particle &p = *it;

    // Update life
    p.life -= deltaTime;

    // Check if particle should be removed
    bool shouldRemove = false;
    if (emitter && emitter->getBehaviour()) {
      shouldRemove = !emitter->getBehaviour()->isAlive(p, model);
    } else {
      shouldRemove = p.life <= 0.0f;
    }

    if (shouldRemove) {
      // Add to pool for reuse
      particlePool.push(p);
      it = activeParticles.erase(it);
    } else {
      // Update particle behaviour
      if (emitter && emitter->getBehaviour()) {
        emitter->getBehaviour()->update(p, deltaTime, model);
      }

      // Update position
      p.position += p.velocity * deltaTime;
      ++it;
    }
  }

  // Automatic emission
  if (emitter) {
    int emitCount = emitter->updateEmission(deltaTime);
    for (int i = 0; i < emitCount && activeParticles.size() < maxParticles;
         ++i) {
      emitParticle();
    }
  }

  // Update buffer data
  updateBuffers();
}

void ParticleSystem::render(Shader &shader, const glm::mat4 &model,
                            const glm::mat4 &view,
                            const glm::mat4 &projection) {
  if (activeParticles.empty())
    return;

  shader.use();

  { // Uniforms
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);
    shader.setMat4("model", model);
  }

  { // Bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, particleTexture);
    shader.setInt("particleTexture", 0);
  }

  // Enable blending for transparency
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Update buffer data before rendering
  updateBuffers();

  // Render particles
  glBindVertexArray(VAO);
  glDrawArrays(GL_POINTS, 0, activeParticles.size());
  glBindVertexArray(0);

  // Disable blending
  glDisable(GL_BLEND);
}

void ParticleSystem::emitParticle() {
  if (activeParticles.size() >= maxParticles) {
    // Maximum particles exceeded
    return;
  }

  auto p = [this] {
    if (particlePool.empty()) {
      // Create new particle if empty
      return Particle{};
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

void ParticleSystem::emitBurst(int count) {
  for (int i = 0; i < count && activeParticles.size() < maxParticles; ++i) {
    emitParticle();
  }
}

void ParticleSystem::reset() {
  // Move all active particles to the pool
  for (const auto &p : activeParticles) {
    particlePool.push(p);
  }
  activeParticles.clear();
}

void ParticleSystem::setupBuffers() {
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, maxParticles * sizeof(Particle), nullptr,
               GL_DYNAMIC_DRAW);

  // Position attribute
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void *)0);

  // Color attribute
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Particle),
                        (void *)offsetof(Particle, color));

  // Size attribute
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Particle),
                        (void *)offsetof(Particle, size));

  glBindVertexArray(0);
}

void ParticleSystem::loadTexture() {
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

void ParticleSystem::cleanup() {
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
  std::queue<Particle> empty;
  std::swap(particlePool, empty);
}

void ParticleSystem::updateBuffers() {
  if (activeParticles.empty())
    return;

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferSubData(GL_ARRAY_BUFFER, 0, activeParticles.size() * sizeof(Particle),
                  activeParticles.data());
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}
