#include "graphics/particles/model_particle_factory.hpp"
#include "graphics/particles/emitters/area_emitter.hpp"
#include "graphics/particles/updaters/aabb_collision_updater.hpp"
#include "graphics/model_factory.hpp"
#include "graphics/particles/updaters/gravity_updater.hpp"
#include <iostream>

namespace graphics::particles {

std::shared_ptr<ModelParticleSystem> ModelParticleFactory::createCubeTestSystem(
    std::shared_ptr<Shader> shader, const glm::vec3 &position,
    size_t maxParticles, float emissionRate, float cubeSize,
    const glm::vec3 &cubeColor,
    const std::vector<GameObject *> &collisionTargets) {
  // Create cube model
  Material cubeMaterial;
  cubeMaterial.ambient = cubeColor;
  cubeMaterial.diffuse = cubeColor;
  cubeMaterial.specular = glm::vec3(0.5f, 0.5f, 0.5f);
  cubeMaterial.shininess = 32.0f;

  auto cubeModelWithMaterials =
      ModelFactory::createCube(cubeSize, cubeMaterial);

  // Create gravity updater
  auto gravityUpdater =
      std::make_shared<GravityUpdater>(glm::vec3{0.0f, -9.81f, 0.0f});

  // Create area emitter
  auto cubeEmitter = std::make_shared<AreaEmitter>(
      nullptr, // default, TODO: improve signature
      position, glm::vec3{0.1f, 0.0f, 0.1f}, nullptr);
  cubeEmitter->setEmissionRate(emissionRate);
  cubeEmitter->setBurst(5, 0.0f); // Initial burst

  // Create model particle system
  auto cubeParticleSystem = std::make_shared<ModelParticleSystem>(
      shader, cubeEmitter, std::move(cubeModelWithMaterials),
      SimulationSpace::WORLD, nullptr);
  cubeParticleSystem->setMaxParticles(maxParticles);
  cubeParticleSystem->addUpdater(gravityUpdater);
  cubeParticleSystem->addDeathCheck(cubeParticleSystem->defaultDeathCheck());
  cubeParticleSystem->init();
  cubeParticleSystem->toggle(true);

  // If collision targets not empty, create collision updater
  if (!collisionTargets.empty()) {
    // Create collision updater
    auto collisionUpdater = std::make_shared<ModelParticleAABBCollisionUpdater>(
        cubeParticleSystem.get(), collisionTargets, 0.01f);

    cubeParticleSystem->addUpdater(collisionUpdater);
  }

  std::println(
      std::clog,
      "Cube test system created at position ({}, {}, {}) with {} max particles",
      position.x, position.y, position.z, maxParticles);

  return cubeParticleSystem;
}
} // namespace graphics::particles
