#include "graphics/particles/emitters/sphere_surface_emitter.hpp"
#include "graphics/particles/particle_factory.hpp"
// #include "graphics/particles/behaviours/collision_behaviour.hpp"
// #include "graphics/particles/behaviours/gravity_behaviour.hpp"
// #include "graphics/particles/behaviours/wind_behaviour.hpp"
#include "graphics/particles/emitters/area_emitter.hpp"
#include "graphics/particles/initializers/base_initializer.hpp"
#include "graphics/particles/initializers/orbital_initializer.hpp"
#include "graphics/particles/updaters/collision_updater.hpp"
#include "graphics/particles/updaters/gravity_updater.hpp"
#include "graphics/particles/updaters/orbital_updater.hpp"
#include "graphics/particles/updaters/wind_updater.hpp"
#include "math/random.hpp"
#include <glm/glm.hpp>

namespace graphics::particles {

std::unique_ptr<ParticleSystem<Particle>> ParticleFactory::createRainSystem(
    std::shared_ptr<TerrainMesh> terrain, const glm::mat4 &terrainModel,
    std::shared_ptr<Shader> shader, const glm::vec3 &position,
    size_t maxParticles, float emissionRate, const GameObject *parent) {

  // const auto gravityBehaviour =
  //     std::make_shared<GravityBehaviour>(glm::vec3(0.0f, -9.81f, 0.0f));
  // const auto dissolveBehaviour = [&terrain, &terrainModel] {
  //   if (terrain) {
  //     return std::make_shared<DissolveBehaviour>(terrain, terrainModel);
  //   } else {
  //     return std::shared_ptr<DissolveBehaviour>(nullptr);
  //   }
  // }();

  const auto gravityUpdater =
      std::make_shared<GravityUpdater>(glm::vec3(0.0f, -9.81f, 0.0f));
  const auto dissolveUpdater = [&terrain, &terrainModel] {
    if (terrain) {
      return std::make_shared<DissolveUpdater>(terrain, terrainModel);
    } else {
      return std::shared_ptr<DissolveUpdater>(nullptr);
    }
  }();

  class RainInitializer : public BaseInitializer {
    virtual void initialize(Particle &particle) override {
      particle.position = glm::vec3(0.0f);
      particle.velocity = glm::vec3((math::uniformDist() - 0.5f) * 0.05f,
                                    -math::uniformDist() * 0.1f - 0.5f,
                                    (math::uniformDist() - 0.5f) * 0.05f);
      particle.color = glm::vec4(0.8f, 0.8f, 1.0f, 0.8f);
      particle.life = math::uniformDist() * 2.0f + 1.0f;
      particle.size = 0.05f + math::uniformDist() * 0.05f;
    }
  };

  /// RainBehaviour: A composition of GravityBehaviour and DissolveBehaviour
  /// WARN: Deprecated
  // class RainBehaviour : public BaseBehaviour {
  // public:
  //   RainBehaviour(std::shared_ptr<GravityBehaviour> gravity,
  //                 std::shared_ptr<DissolveBehaviour> dissolve)
  //       : gravityBehaviour{gravity}, dissolveBehaviour{dissolve} {}
  //
  //   virtual void initialize(Particle &particle) override {
  //     particle.position = glm::vec3(0.0f);
  //     particle.velocity = glm::vec3((uniformDist(gen) - 0.5f) * 0.05f,
  //                                   -uniformDist(gen) * 0.1f - 0.5f,
  //                                   (uniformDist(gen) - 0.5f) * 0.05f);
  //     particle.color = glm::vec4(0.8f, 0.8f, 1.0f, 0.8f);
  //     particle.life = uniformDist(gen) * 2.0f + 1.0f;
  //     particle.size = 0.05f + uniformDist(gen) * 0.05f;
  //   }
  //
  //   virtual void update(Particle &particle, float deltaTime,
  //                       const glm::mat4 &model,
  //                       SimulationSpace space) override {
  //     if (gravityBehaviour) {
  //       gravityBehaviour->update(particle, deltaTime, model, space);
  //     }
  //     if (dissolveBehaviour) {
  //       dissolveBehaviour->update(particle, deltaTime, model, space);
  //     }
  //   }
  //
  //   virtual bool isAlive(const Particle &particle, const glm::mat4 &model,
  //                        SimulationSpace space) const override {
  //     if (dissolveBehaviour) {
  //       return dissolveBehaviour->isAlive(particle, model, space);
  //     }
  //     return BaseBehaviour::isAlive(particle, model, space) &&
  //            particle.position.y > -10.0f;
  //   }
  //
  // private:
  //   std::shared_ptr<GravityBehaviour> gravityBehaviour;
  //   std::shared_ptr<DissolveBehaviour> dissolveBehaviour;
  // };

  // auto rainBehaviour =
  //     std::make_shared<RainBehaviour>(gravityBehaviour, dissolveBehaviour);

  auto rainInitializer = std::make_shared<RainInitializer>();

  // Create emitter
  auto emitter = std::make_shared<AreaEmitter>(
      rainInitializer, position, glm::vec3(0.1f, 0.0f, 0.1f), parent);
  emitter->setEmissionRate(emissionRate);
  emitter->setBurst(100, 5.0f);

  // Create particle system
  // Rain system in world space
  auto system = std::make_unique<ParticleSystem<Particle>>(
      shader, emitter, SimulationSpace::WORLD, nullptr);

  system->addUpdater(gravityUpdater);
  system->addUpdater(dissolveUpdater);

  // TODO: This should be default, can use a config in ParticleSystem
  system->addDeathCheck(system->defaultDeathCheck());

  system->setMaxParticles(maxParticles);
  system->init();

  return system;
}

std::unique_ptr<ParticleSystem<Particle>> ParticleFactory::createSnowSystem(
    std::shared_ptr<TerrainMesh> terrain, const glm::mat4 &terrainModel,
    std::shared_ptr<Shader> shader, const glm::vec3 &position,
    size_t maxParticles, float emissionRate, const GameObject *parent) {

  // const auto gravityBehaviour =
  //     std::make_shared<GravityBehaviour>(glm::vec3(0.0f, -0.05f, 0.0f));
  // const auto windBehaviour =
  //     std::make_shared<WindBehaviour>(glm::vec3{0.0, 0.0, 0.0}, 0.15f);
  // const auto coverBehaviour = [&terrain, &terrainModel] {
  //   if (terrain) {
  //     return std::make_shared<CoverBehaviour>(terrain, terrainModel);
  //   } else {
  //     return std::shared_ptr<CoverBehaviour>(nullptr);
  //   }
  // }();

  const auto gravityUpdater =
      std::make_shared<GravityUpdater>(glm::vec3(0.0f, -0.05f, 0.0f));
  const auto windUpdater =
      std::make_shared<WindUpdater>(glm::vec3{0.0, 0.0, 0.0}, 0.15f);
  const auto coverUpdater = [&terrain, &terrainModel] {
    if (terrain) {
      return std::make_shared<CoverUpdater>(terrain, terrainModel);
    } else {
      return std::shared_ptr<CoverUpdater>(nullptr);
    }
  }();

  class SnowInitializer : public BaseInitializer {
    virtual void initialize(Particle &particle) override {
      particle.position = glm::vec3(0.0f);
      particle.velocity = glm::vec3((math::uniformDist() - 0.5f) * 0.02f,
                                    -math::uniformDist() * 0.1f - 0.05f,
                                    (math::uniformDist() - 0.5f) * 0.02f);
      particle.color = glm::vec4(1.0f, 1.0f, 1.0f, 0.8f);
      particle.life = math::uniformDist() * 20.0f + 10.0f;
      particle.size = 1.0f + math::uniformDist() * 1.0f;
    }
  };

  /// SnowBehaviour: A composition of GravityBehaviour, WindBehaviour and
  /// CoverBehaviour
  /// WARN: Deprecated
  // class SnowBehaviour : public BaseBehaviour {
  // public:
  //   SnowBehaviour(std::shared_ptr<GravityBehaviour> gravity,
  //                 std::shared_ptr<CoverBehaviour> cover,
  //                 std::shared_ptr<WindBehaviour> wind)
  //       : gravityBehaviour(gravity), coverBehaviour(cover),
  //         windBehaviour(wind) {}
  //
  //   virtual void initialize(Particle &particle) override {
  //     particle.position = glm::vec3(0.0f);
  //     particle.velocity = glm::vec3((uniformDist(gen) - 0.5f) * 0.02f,
  //                                   -uniformDist(gen) * 0.1f - 0.05f,
  //                                   (uniformDist(gen) - 0.5f) * 0.02f);
  //     particle.color = glm::vec4(1.0f, 1.0f, 1.0f, 0.8f);
  //     particle.life = uniformDist(gen) * 20.0f + 10.0f;
  //     particle.size = 1.0f + uniformDist(gen) * 1.0f;
  //   }
  //
  //   virtual void update(Particle &particle, float deltaTime,
  //                       const glm::mat4 &model,
  //                       SimulationSpace space) override {
  //     if (gravityBehaviour) {
  //       gravityBehaviour->update(particle, deltaTime, model, space);
  //     }
  //
  //     if (coverBehaviour) {
  //       coverBehaviour->update(particle, deltaTime, model, space);
  //     }
  //
  //     if (windBehaviour) {
  //       windBehaviour->update(particle, deltaTime, model, space);
  //     }
  //
  //     // Add waving motion
  //     const auto waveFrequency = 2.5f;
  //     const auto waveAmplitude = 0.05f;
  //     particle.velocity.x +=
  //         std::sin(particle.life * waveFrequency) * waveAmplitude *
  //         deltaTime;
  //     particle.velocity.z +=
  //         std::cos(particle.life * waveFrequency) * waveAmplitude *
  //         deltaTime;
  //   }
  //
  //   virtual bool isAlive(const Particle &particle, const glm::mat4 &model,
  //                        SimulationSpace space) const override {
  //     if (coverBehaviour) {
  //       return coverBehaviour->isAlive(particle, model, space);
  //     }
  //     return BaseBehaviour::isAlive(particle, model, space) &&
  //            particle.position.y > -10.0f;
  //   }
  //
  // private:
  //   std::shared_ptr<GravityBehaviour> gravityBehaviour;
  //   std::shared_ptr<CoverBehaviour> coverBehaviour;
  //   std::shared_ptr<WindBehaviour> windBehaviour;
  // };

  // auto snowBehaviour = std::make_shared<SnowBehaviour>(
  //     gravityBehaviour, coverBehaviour, windBehaviour);

  auto snowInitializer = std::make_shared<SnowInitializer>();

  // Create emitter
  auto emitter = std::make_shared<AreaEmitter>(
      snowInitializer, position, glm::vec3(0.1f, 0.0f, 0.1f), parent);
  emitter->setEmissionRate(emissionRate);
  emitter->setBurst(50, 8.0f);

  // Create particle system
  // Snow system in world space
  auto system = std::make_unique<ParticleSystem<Particle>>(
      shader, emitter, SimulationSpace::WORLD, nullptr);
  system->setMaxParticles(maxParticles);

  system->addUpdater(gravityUpdater);
  system->addUpdater(coverUpdater);
  system->addUpdater(windUpdater);

  // TODO: This should be default, can use a config in ParticleSystem
  system->addDeathCheck(system->defaultDeathCheck());

  system->init();

  return system;
}

std::unique_ptr<ParticleSystem<Particle>> ParticleFactory::createOrbAuraSystem(
    GameObject *const parent, std::shared_ptr<Shader> shader,
    const glm::vec4 &glowColor, float intensity, int maxParticles) {

  // Create orbital behaviour
  const auto minRadius = 0.2f;
  const auto maxRadius = 0.5f;
  const OrbitalConfig config{
      .center = parent->position,
      .glowColor = glowColor,
      .orbitSpeed = 0.2f * intensity,
      .minRadius = minRadius,
      .maxRadius = maxRadius,
  };
  auto orbitalInitializer =
      std::make_shared<OrbitalParticleInitializer>(config);
  auto orbitalUpdater = std::make_shared<OrbitalParticleUpdater>(config);

  // auto orbitalBehaviour = std::make_shared<OrbitalParticleBehaviour>(
  //     parent->position, // center
  //     glowColor,        // color
  //     0.2f * intensity, // orbit speed scaled by intensity
  //     minRadius,        // min radius
  //     maxRadius         // max radius
  // );

  // Create sphere surface emitter
  float avgRadius = (minRadius + maxRadius) / 2.0f;
  auto emitter = std::make_shared<SphereSurfaceEmitter>(orbitalInitializer,
                                                        parent, avgRadius);
  emitter->setEmissionRate(3000.0f);
  emitter->setBurst(0, 0);

  // Create particle system
  // Orb aura should use LOCAL space to follow parent movement
  auto system = std::make_unique<ParticleSystem<Particle>>(
      shader, emitter, SimulationSpace::LOCAL, nullptr);
  system->setMaxParticles(maxParticles);

  // TODO: This should be default, can use a config in ParticleSystem
  system->addDeathCheck(system->defaultDeathCheck());
  system->addUpdater(orbitalUpdater);

  system->init();

  return system;
}
} // namespace graphics::particles
