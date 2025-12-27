#include "graphics/particles/model_particle_factory.hpp"
#include "graphics/particles/emitters/area_emitter.hpp"
#include "graphics/particles/behaviours/gravity_behaviour.hpp"
#include "graphics/model_factory.hpp"
#include <iostream>

namespace graphics::particles {

std::shared_ptr<ModelParticleSystem> ModelParticleFactory::createCubeTestSystem(
    const glm::vec3 &position, size_t maxParticles, float emissionRate,
    float cubeSize, const glm::vec3 &cubeColor) {
  // Create cube model
  Material cubeMaterial;
  cubeMaterial.ambient = cubeColor;
  cubeMaterial.diffuse = cubeColor;
  cubeMaterial.specular = glm::vec3(0.5f, 0.5f, 0.5f);
  cubeMaterial.shininess = 32.0f;

  auto cubeModelWithMaterials =
      ModelFactory::createCube(cubeSize, cubeMaterial);
  auto cubeModel =
      std::shared_ptr<Model>(std::move(cubeModelWithMaterials.model));

  // Use first material from loaded model or create default
  std::shared_ptr<Material> particleMaterial;
  if (!cubeModelWithMaterials.materials.empty()) {
    particleMaterial =
        std::make_shared<Material>(cubeModelWithMaterials.materials[0]);
  } else {
    particleMaterial = std::make_shared<Material>(cubeMaterial);
  }

  // Create gravity behaviour
  auto gravityBehaviour =
      std::make_shared<GravityBehaviour>(glm::vec3(0.0f, -9.81f, 0.0f));

  // Create area emitter
  auto cubeEmitter = std::make_shared<AreaEmitter>(
      gravityBehaviour, position, glm::vec3{0.1f, 0.0f, 0.1f}, nullptr);
  cubeEmitter->setEmissionRate(emissionRate);
  cubeEmitter->setBurst(5, 0.0f); // Initial burst

  // Create model particle system
  auto cubeParticleSystem = std::make_shared<ModelParticleSystem>(
      cubeEmitter, SimulationSpace::WORLD, nullptr, cubeModel,
      particleMaterial);
  cubeParticleSystem->setMaxParticles(maxParticles);
  cubeParticleSystem->init();
  cubeParticleSystem->toggle(true);

  std::println(
      std::clog,
      "Cube test system created at position ({}, {}, {}) with {} max particles",
      position.x, position.y, position.z, maxParticles);

  return cubeParticleSystem;
}
} // namespace graphics::particles
