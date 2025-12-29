#include "graphics/particles/arrow_launcher.hpp"
// #include "graphics/particles/behaviours/arrow_collision_behaviour.hpp"
// #include "graphics/particles/behaviours/arrow_physics_behaviour.hpp"
#include "graphics/particles/initializers/arrow_initializer.hpp"
#include "graphics/particles/updaters/arrow_collision_updater.hpp"
#include "graphics/particles/updaters/arrow_physics_updater.hpp"
#include "scene/game_manager.hpp"
#include <algorithm>
#include <iostream>
#include <print>

namespace graphics::particles {

ArrowLauncher::ArrowLauncher(std::shared_ptr<GameManager> gameManager)
    : gameManager{gameManager} {}

void ArrowLauncher::init(ModelWithMaterials arrowModelWithMaterials,
                         std::shared_ptr<Shader> arrowShader,
                         size_t maxArrows) {
  if (initialized) {
    std::println(std::cerr, "ArrowLauncher already initialized");
    return;
  }

  { // behaviours
    // arrowBehaviour is created locally in setEmitter()
    // collisionBehaviour = std::make_shared<ArrowCollisionBehaviour>();
    collisionUpdater = std::make_shared<ArrowCollisionUpdater>();
  }

  { // Create arrow system without emitter initially
    arrowSystem = std::make_shared<ModelParticleSystem>(
        arrowShader,
        // Emitter will be set via setEmitter()
        nullptr, std::move(arrowModelWithMaterials), SimulationSpace::WORLD,
        nullptr // No custom transform
    );

    arrowSystem->setMaxParticles(maxArrows);
    arrowSystem->init();
  }

  initialized = true;
  std::println(std::clog, "ArrowLauncher initialized with max {} arrows",
               maxArrows);
}

void ArrowLauncher::setEmitter(const glm::vec3 &position,
                               const glm::vec3 &direction, float speed,
                               float spreadAngle) {

  if (not initialized) {
    std::println(std::cerr, "ArrowLauncher not initialized");
    return;
  }

  // Composite behaviour for emitter (arrow physics + collision)
  // WARN: Deprecated
  // class ArrowBehaviour : public ParticleBehaviour {
  // public:
  //   ArrowBehaviour(std::shared_ptr<ArrowPhysicsBehaviour> arrow,
  //                  std::shared_ptr<ArrowCollisionBehaviour> collision)
  //       : arrowBehaviour{arrow}, collisionBehaviour{collision} {}
  //
  //   void initialize(Particle &particle) override {
  //     if (arrowBehaviour) {
  //       arrowBehaviour->initialize(particle);
  //     }
  //   }
  //
  //   void update(Particle &particle, float deltaTime, const glm::mat4 &model,
  //               SimulationSpace space) override {
  //     if (arrowBehaviour) {
  //       arrowBehaviour->update(particle, deltaTime, model, space);
  //     }
  //     if (collisionBehaviour) {
  //       collisionBehaviour->update(particle, deltaTime, model, space);
  //     }
  //   }
  //
  //   bool isAlive(const Particle &particle, const glm::mat4 &model,
  //                SimulationSpace space) const override {
  //     bool alive = true;
  //     if (arrowBehaviour) {
  //       alive = alive && arrowBehaviour->isAlive(particle, model, space);
  //     }
  //     if (collisionBehaviour) {
  //       alive = alive && collisionBehaviour->isAlive(particle, model, space);
  //     }
  //     return alive;
  //   }
  //
  //   void onDeath(Particle &particle) override {
  //     if (arrowBehaviour) {
  //       arrowBehaviour->onDeath(particle);
  //     }
  //   }
  //
  //   void onRespawn(Particle &particle) override {
  //     if (arrowBehaviour) {
  //       arrowBehaviour->onRespawn(particle);
  //     }
  //   }
  //
  // private:
  //   std::shared_ptr<ArrowPhysicsBehaviour> arrowBehaviour;
  //   std::shared_ptr<ArrowCollisionBehaviour> collisionBehaviour;
  // };

  // auto arrowBehaviour = createArrowPhysicsBehaviour(1.0f, 0.01f);
  // auto compositeBehaviour =
  //     std::make_shared<ArrowBehaviour>(arrowBehaviour, collisionBehaviour);

  auto arrowInitializer = std::make_shared<ArrowInitializer>();

  auto arrowPhysicsUpdater = std::make_shared<ArrowPhysicsUpdater>();

  // Always create new emitter with updated parameters
  emitter = std::make_shared<ArrowEmitter>(
      arrowInitializer, position, direction, speed, spreadAngle,
      nullptr // No parent GameObject (world space)
  );
  emitter->setEmissionRate(0.0f);
  arrowSystem->emitter = emitter;
  arrowSystem->addUpdater(collisionUpdater);
  arrowSystem->addUpdater(arrowPhysicsUpdater);

  std::println(std::clog,
               "Arrow emitter set at ({}, {}, {}) direction ({}, {}, {})",
               position.x, position.y, position.z, direction.x, direction.y,
               direction.z);
}

void ArrowLauncher::launch(size_t arrowsPerEmitter) {
  if (not initialized or not emitter) {
    return;
  }

  emitter->setBurst(arrowsPerEmitter, 0.0f); // Fire immediately
  arrowSystem->toggle(true);

  std::println("Fired {} arrows", arrowsPerEmitter);
}

void ArrowLauncher::startContinuousFire(float arrowsPerSecond) {
  if (not initialized or not emitter) {
    return;
  }

  emitter->setEmissionRate(arrowsPerSecond);
  arrowSystem->toggle(true);

  std::println("Started continuous fire at {} arrows/sec", arrowsPerSecond);
}

void ArrowLauncher::stopContinuousFire() {
  if (not initialized or not emitter) {
    return;
  }

  emitter->setEmissionRate(0.0f);
  arrowSystem->toggle(false);

  std::println("Stopped continuous fire");
}

void ArrowLauncher::addCollisionTarget(GameObject *target) {
  // if (not target or not collisionBehaviour) {
  if (not target or not collisionUpdater) {
    return;
  }

  // collisionBehaviour->addCollisionTarget(target);
  collisionUpdater->addCollisionTarget(target);
  collisionTargets.push_back(target);

  std::println("Added collision target: {}", target->getName());
}

void ArrowLauncher::removeCollisionTarget(GameObject *target) {
  if (not collisionUpdater) {
    return;
  }

  collisionUpdater->removeCollisionTarget(target);

  // Remove from local list
  auto it = std::find(collisionTargets.begin(), collisionTargets.end(), target);
  if (it != collisionTargets.end()) {
    collisionTargets.erase(it);
  }
}

void ArrowLauncher::clearCollisionTargets() {
  if (collisionUpdater) {
    collisionUpdater->clearCollisionTargets();
  }
  collisionTargets.clear();
}

void ArrowLauncher::update(float deltaTime) {
  if (not initialized or not arrowSystem) {
    return;
  }
  arrowSystem->update(deltaTime);
}
} // namespace graphics::particles
