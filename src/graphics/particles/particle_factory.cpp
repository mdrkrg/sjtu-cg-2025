#include "particles/particle_factory.hpp"
#include "particles/behaviours/collision_behaviour.hpp"
#include "particles/behaviours/gravity_behaviour.hpp"
#include "particles/behaviours/wind_behaviour.hpp"
#include "particles/emitters/area_emitter.hpp"
#include <glm/glm.hpp>

std::unique_ptr<ParticleSystem> ParticleFactory::createRainSystem(
    std::shared_ptr<TerrainMesh> terrain, const glm::mat4 &terrainModel,
    const glm::vec3 &position, size_t maxParticles, float emissionRate) {

  const auto gravityBehaviour =
      std::make_shared<GravityBehaviour>(glm::vec3(0.0f, -9.81f, 0.0f));
  const auto dissolveBehaviour = [&terrain, &terrainModel] {
    if (terrain) {
      return std::make_shared<DissolveBehaviour>(terrain, terrainModel);
    } else {
      return std::shared_ptr<DissolveBehaviour>(nullptr);
    }
  }();

  /// RainBehaviour: A composition of GravityBehaviour and DissolveBehaviour
  class RainBehaviour : public BaseBehaviour {
  public:
    RainBehaviour(std::shared_ptr<GravityBehaviour> gravity,
                  std::shared_ptr<DissolveBehaviour> dissolve)
        : gravityBehaviour{gravity}, dissolveBehaviour{dissolve} {}

    virtual void initialize(Particle &particle) override {
      particle.position = glm::vec3(0.0f);
      particle.velocity = glm::vec3((uniformDist(gen) - 0.5f) * 2.0f,
                                    -uniformDist(gen) * 5.0f - 5.0f,
                                    (uniformDist(gen) - 0.5f) * 2.0f);
      particle.color = glm::vec4(0.8f, 0.8f, 1.0f, 0.8f);
      particle.life = uniformDist(gen) * 2.0f + 1.0f;
      particle.size = 0.05f + uniformDist(gen) * 0.05f;
    }

    virtual void update(Particle &particle, float deltaTime,
                        const glm::mat4 &model) override {
      if (gravityBehaviour) {
        gravityBehaviour->update(particle, deltaTime, model);
      }
      if (dissolveBehaviour) {
        dissolveBehaviour->update(particle, deltaTime, model);
      }
    }

    virtual bool isAlive(const Particle &particle,
                         const glm::mat4 &model) const override {
      if (dissolveBehaviour) {
        return dissolveBehaviour->isAlive(particle, model);
      }
      return BaseBehaviour::isAlive(particle, model) &&
             particle.position.y > -10.0f;
    }

  private:
    std::shared_ptr<GravityBehaviour> gravityBehaviour;
    std::shared_ptr<DissolveBehaviour> dissolveBehaviour;
  };

  auto rainBehaviour =
      std::make_shared<RainBehaviour>(gravityBehaviour, dissolveBehaviour);

  // Create emitter
  auto emitter = std::make_shared<AreaEmitter>(rainBehaviour, position,
                                               glm::vec3(10.0f, 0.0f, 10.0f));
  emitter->setEmissionRate(emissionRate);
  emitter->setBurst(100, 5.0f);

  // Create particle system
  auto system = std::make_unique<ParticleSystem>(emitter);
  system->setMaxParticles(maxParticles);
  system->init();

  return system;
}

std::unique_ptr<ParticleSystem> ParticleFactory::createSnowSystem(
    std::shared_ptr<TerrainMesh> terrain, const glm::mat4 &terrainModel,
    const glm::vec3 &position, size_t maxParticles, float emissionRate) {

  const auto gravityBehaviour =
      std::make_shared<GravityBehaviour>(glm::vec3(0.0f, -1.5f, 0.0f));
  const auto windBehaviour =
      std::make_shared<WindBehaviour>(glm::vec3{0.0, 0.0, 0.0}, 2.0f);
  const auto coverBehaviour = [&terrain, &terrainModel] {
    if (terrain) {
      return std::make_shared<CoverBehaviour>(terrain, terrainModel);
    } else {
      return std::shared_ptr<CoverBehaviour>(nullptr);
    }
  }();

  /// SnowBehaviour: A composition of GravityBehaviour, WindBehaviour and
  /// CoverBehaviour
  class SnowBehaviour : public BaseBehaviour {
  public:
    SnowBehaviour(std::shared_ptr<GravityBehaviour> gravity,
                  std::shared_ptr<CoverBehaviour> cover,
                  std::shared_ptr<WindBehaviour> wind)
        : gravityBehaviour(gravity), coverBehaviour(cover),
          windBehaviour(wind) {}

    virtual void initialize(Particle &particle) override {
      particle.position = glm::vec3(0.0f);
      particle.velocity = glm::vec3((uniformDist(gen) - 0.5f) * 0.5f,
                                    -uniformDist(gen) * 0.5f - 0.5f,
                                    (uniformDist(gen) - 0.5f) * 0.5f);
      particle.color = glm::vec4(1.0f, 1.0f, 1.0f, 0.8f);
      particle.life = uniformDist(gen) * 20.0f + 10.0f;
      particle.size = 1.0f + uniformDist(gen) * 1.0f;
    }

    virtual void update(Particle &particle, float deltaTime,
                        const glm::mat4 &model) override {
      if (gravityBehaviour) {
        gravityBehaviour->update(particle, deltaTime, model);
      }

      if (coverBehaviour) {
        coverBehaviour->update(particle, deltaTime, model);
      }

      if (windBehaviour) {
        windBehaviour->update(particle, deltaTime, model);
      }

      // Add waving motion
      const auto waveFrequency = 2.5f;
      const auto waveAmplitude = 0.05f;
      particle.velocity.x +=
          std::sin(particle.life * waveFrequency) * waveAmplitude * deltaTime;
      particle.velocity.z +=
          std::cos(particle.life * waveFrequency) * waveAmplitude * deltaTime;
    }

    virtual bool isAlive(const Particle &particle,
                         const glm::mat4 &model) const override {
      if (coverBehaviour) {
        return coverBehaviour->isAlive(particle, model);
      }
      return BaseBehaviour::isAlive(particle, model) &&
             particle.position.y > -10.0f;
    }

  private:
    std::shared_ptr<GravityBehaviour> gravityBehaviour;
    std::shared_ptr<CoverBehaviour> coverBehaviour;
    std::shared_ptr<WindBehaviour> windBehaviour;
  };

  auto snowBehaviour = std::make_shared<SnowBehaviour>(
      gravityBehaviour, coverBehaviour, windBehaviour);

  // Create emitter
  auto emitter = std::make_shared<AreaEmitter>(snowBehaviour, position,
                                               glm::vec3(10.0f, 0.0f, 10.0f));
  emitter->setEmissionRate(emissionRate);
  emitter->setBurst(50, 8.0f);

  // Create particle system
  auto system = std::make_unique<ParticleSystem>(emitter);
  system->setMaxParticles(maxParticles);
  system->init();

  return system;
}
