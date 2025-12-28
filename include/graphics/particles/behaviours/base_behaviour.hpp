#pragma once

#include "graphics/particles/particle_behaviour.hpp"
#include <glm/glm.hpp>
#include <random>

class BaseBehaviour : public ParticleBehaviour {
public:
  BaseBehaviour() = default;
  virtual ~BaseBehaviour() = default;

  /// Default implementation for particle initialization
  virtual void initialize(Particle &particle) override {
    particle.position = glm::vec3(0.0f);
    particle.velocity = glm::vec3(0.0f);
    particle.color = glm::vec4(1.0f);
    particle.life = 1.0f;
    particle.size = 1.0f;
  }

  /// Default implementation for particle update
  virtual void update(Particle &particle, float deltaTime,
                      const glm::mat4 &model, SimulationSpace space) override {
    // Empty, subclasses can override
    (void)particle;
    (void)deltaTime;
    (void)model;
    (void)space;
  }

  /// Default implementation for alive check
  virtual bool isAlive(const Particle &particle, const glm::mat4 &model,
                       SimulationSpace space) const override {
    (void)model;
    (void)space;
    return particle.life > 0.0f;
  }

protected:
  static std::random_device rd;
  static std::mt19937 gen;
  static std::uniform_real_distribution<float> uniformDist;
};

inline std::random_device BaseBehaviour::rd{};
inline std::mt19937 BaseBehaviour::gen{rd()};
inline std::uniform_real_distribution<float> BaseBehaviour::uniformDist{0.0f,
                                                                        1.0f};
