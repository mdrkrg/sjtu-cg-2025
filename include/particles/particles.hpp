#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <shader.h>
#include <terrain_mesh.hpp>

#include <vector>

struct Particle {
  glm::vec3 position;
  glm::vec3 velocity;
  glm::vec4 color;
  float life;
  float size;
};

class ParticleSystem {
public:
  ParticleSystem(size_t size);
  virtual ~ParticleSystem();

  virtual void init(const glm::mat4 &model);
  virtual void update(float deltaTime, const glm::mat4 &model);
  virtual void render(Shader &shader, const glm::mat4 &model,
                      const glm::mat4 &view, const glm::mat4 &projection);
  virtual void reset(const glm::mat4 &model);

  virtual glm::vec3 genPos() = 0;
  virtual glm::vec3 genVel() = 0;
  virtual float genSize() = 0;
  virtual float genLife() = 0;

  virtual glm::vec4 color() = 0;

  virtual bool shouldRespawn(const Particle &p) const { return p.life <= 0; }

  size_t size() { return particles.size(); }

  void loadTexture();

  virtual void updateParticle(Particle &, float, const glm::mat4 &) {}

protected:
  std::vector<Particle> particles;

  // OpenGL resources
  unsigned int VAO, VBO;
  unsigned int particleTexture;

  void respawnParticle(Particle &p, const glm::mat4 &model);

private:
  void setupBuffers();
};
#endif
