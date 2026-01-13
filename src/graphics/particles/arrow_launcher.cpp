#include "graphics/particles/arrow_launcher.hpp"
#include "graphics/particles/emitters/arrow_emitter.hpp"
#include "graphics/particles/initializers/arrow_initializer.hpp"
#include "graphics/particles/updaters/arrow_collision_updater.hpp"
#include "graphics/particles/updaters/arrow_physics_updater.hpp"
#include "graphics/particles/death_checks/arrow_collision_death_check.hpp"
#include "scene/game_manager.hpp"
#include <iostream>
#include <print>

namespace graphics::particles {

ArrowLauncher::ArrowLauncher(std::shared_ptr<GameManager> gameManager,
                             EmissionPolicy emissionPolicy)
    : gameManager{gameManager}, arrowModelWithMaterials{nullptr, {}},
      arrowShader{nullptr} {
  // Create emitter group with specified policy
  emitterGroup =
      std::make_shared<EmitterGroup<ModelParticleSystem>>(emissionPolicy);
}

void ArrowLauncher::init(ModelWithMaterials arrowModelWithMaterials,
                         std::shared_ptr<Shader> arrowShader,
                         size_t maxArrows) {
  if (initialized) {
    std::println(std::cerr, "ArrowLauncher already initialized");
    return;
  }

  // Store resources for creating new systems
  this->arrowModelWithMaterials = std::move(arrowModelWithMaterials);
  this->arrowShader = arrowShader;
  this->maxArrowsPerSystem = maxArrows;

  // Create collision updater (shared across all systems)
  collisionUpdater = std::make_shared<ArrowCollisionUpdater>();

  initialized = true;
  std::println(std::clog,
               "ArrowLauncher initialized with max {} arrows per system",
               maxArrows);
}

void ArrowLauncher::addEmitter(const glm::vec3 &position,
                               const glm::vec3 &direction, float speed,
                               float spreadAngle) {
  if (not initialized) {
    std::println(std::cerr, "ArrowLauncher not initialized");
    return;
  }

  // Create new arrow system for this emitter
  const auto system =
      createArrowSystem(position, direction, speed, spreadAngle);

  // Add to emitter group
  emitterGroup->addSystem(system);

  std::println(std::clog,
               "Added arrow emitter at ({}, {}, {}) direction ({}, {}, {})",
               position.x, position.y, position.z, direction.x, direction.y,
               direction.z);
}

void ArrowLauncher::removeEmitter(size_t index) {
  if (not initialized or not emitterGroup or
      index >= emitterGroup->getSystemCount()) {
    return;
  }

  const auto systemToRemove = emitterGroup->getSystem(index);
  if (systemToRemove) {
    emitterGroup->removeSystem(systemToRemove);
    std::println(std::clog, "Removed emitter at index {}", index);
  }
}

void ArrowLauncher::clearEmitters() {
  if (not initialized or not emitterGroup) {
    return;
  }

  emitterGroup->clearSystems();
  std::println(std::clog, "Cleared all emitters");
}

size_t ArrowLauncher::getEmitterCount() const {
  if (not emitterGroup) {
    return 0;
  }
  return emitterGroup->getSystemCount();
}

void ArrowLauncher::launch(size_t arrowsPerEmitter) {
  if (not initialized or not emitterGroup) {
    return;
  }

  emitterGroup->toggle(true);
  emitterGroup->emitBurst(arrowsPerEmitter);

  std::println("Fired {} arrows from each emitter (total: {} systems)",
               arrowsPerEmitter, emitterGroup->getSystemCount());
}

void ArrowLauncher::startContinuousFire(float arrowsPerSecond) {
  if (not initialized or not emitterGroup) {
    return;
  }

  emitterGroup->startContinuousEmission(arrowsPerSecond);

  std::println("Started continuous fire at {} arrows/sec across {} systems",
               arrowsPerSecond, emitterGroup->getSystemCount());
}

void ArrowLauncher::stopContinuousFire() {
  if (not initialized or not emitterGroup) {
    return;
  }

  emitterGroup->stopContinuousEmission();
  std::println("Stopped continuous fire");
}

void ArrowLauncher::addCollisionTarget(GameObject *target) {
  if (not target or not collisionUpdater) {
    return;
  }

  // Cache the collision meshes
  target->getCollisionMeshes();

  // Add to collision updater (shared across all systems)
  collisionUpdater->addCollisionTarget(target);

  std::println("Added collision target: {}", target->getName());
}

void ArrowLauncher::removeCollisionTarget(GameObject *target) {
  if (not collisionUpdater) {
    return;
  }

  collisionUpdater->removeCollisionTarget(target);
}

void ArrowLauncher::clearCollisionTargets() {
  if (collisionUpdater) {
    collisionUpdater->clearCollisionTargets();
  }
}

void ArrowLauncher::update(float deltaTime) {
  if (not initialized or not emitterGroup) {
    return;
  }
  emitterGroup->update(deltaTime);
}

void ArrowLauncher::render(const glm::mat4 &view, const glm::mat4 &projection) {
  if (not initialized or not emitterGroup) {
    return;
  }
  emitterGroup->render(view, projection);
}

size_t ArrowLauncher::getTotalActiveArrows() const {
  if (not emitterGroup) {
    return 0;
  }
  return emitterGroup->getTotalActiveParticles();
}

void ArrowLauncher::setEmissionPolicy(EmissionPolicy policy) {
  if (emitterGroup) {
    emitterGroup->setEmissionPolicy(policy);
  }
}

std::shared_ptr<ModelParticleSystem> ArrowLauncher::createArrowSystem(
    const glm::vec3 &position, const glm::vec3 &direction, float speed,
    float spreadAngle, float gravity, float drag, float lifetime) {

  // Create arrow initializer
  auto arrowInitializer = std::make_shared<ArrowInitializer>();

  // Create arrow emitter
  auto emitter = std::make_shared<ArrowEmitter>(arrowInitializer, position,
                                                direction, speed, spreadAngle,
                                                nullptr // No parent
  );
  emitter->setEmissionRate(0.0f);

  // Create particle system with this emitter
  auto system =
      std::make_shared<ModelParticleSystem>(arrowShader, emitter,
                                            arrowModelWithMaterials, // Copy
                                            SimulationSpace::WORLD,
                                            nullptr // No custom transform
      );

  system->setMaxParticles(maxArrowsPerSystem);
  system->init();

  // Add updaters
  auto arrowPhysicsUpdater =
      std::make_shared<ArrowPhysicsUpdater>(gravity, drag, lifetime);
  system->addUpdater(arrowPhysicsUpdater);
  system->addUpdater(collisionUpdater);

  // Add death checks
  system->addDeathCheck(system->defaultDeathCheck());
  system->addDeathCheck(std::make_shared<ArrowCollisionDeathCheck>());

  return system;
}
} // namespace graphics::particles
