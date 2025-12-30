#include "graphics/particles/emitters/sphere_surface_emitter.hpp"
#include "graphics/particles/particle_factory.hpp"
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
