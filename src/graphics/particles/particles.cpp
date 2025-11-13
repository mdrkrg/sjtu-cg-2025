#include "particles/particles.hpp"
#include <cmath>

ParticleSystem::ParticleSystem(size_t size) {
  particles.resize(size, Particle{});
}

ParticleSystem::~ParticleSystem() {
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteTextures(1, &particleTexture);
}

void ParticleSystem::init(const glm::mat4 &model) {
  for (auto &p : particles) {
    respawnParticle(p, model);
  }
  setupBuffers();
  loadTexture();
}

void ParticleSystem::setupBuffers() {
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, size() * sizeof(Particle), particles.data(),
               GL_DYNAMIC_DRAW);

  // Position attribute
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void *)0);

  // Color attribute
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Particle),
                        (void *)offsetof(Particle, color));

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

void ParticleSystem::update(float deltaTime, const glm::mat4 &model) {
  for (auto &p : particles) {
    // Update life
    p.life -= deltaTime;

    // Check respawn condition
    if (shouldRespawn(p)) {
      respawnParticle(p, model);
      continue;
    }

    updateParticle(p, deltaTime, model);

    // Update position
    p.position += p.velocity * deltaTime;
  }

  // Update buffer data
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferSubData(GL_ARRAY_BUFFER, 0, size() * sizeof(Particle),
                  particles.data());
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void ParticleSystem::render(Shader &shader, const glm::mat4 &model,
                            const glm::mat4 &view,
                            const glm::mat4 &projection) {
  shader.use();

  // Set uniforms
  shader.setMat4("view", view);
  shader.setMat4("projection", projection);
  shader.setMat4("model", model);

  // Bind texture
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, particleTexture);
  shader.setInt("particleTexture", 0);

  // Enable blending for transparency
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Render particles
  glBindVertexArray(VAO);
  glDrawArrays(GL_POINTS, 0, size());
  glBindVertexArray(0);

  // Disable blending
  glDisable(GL_BLEND);
}

void ParticleSystem::respawnParticle(Particle &p, const glm::mat4 &model) {
  // p.position = glm::vec4(genPos(), 1.0f);
  p.position = genPos();
  // p.velocity = model * glm::vec4(genVel(), 0.0f);
  p.velocity = genVel();
  p.color = color();
  p.size = genSize();
  p.life = genLife();
}

void ParticleSystem::reset(const glm::mat4 &model) { init(model); }
