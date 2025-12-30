#include "graphics/particles/model_particle_factory.hpp"
#include "graphics/particles/emitters/area_emitter.hpp"
// #include "graphics/particles/particle_behaviour.hpp"
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

  // Wrapper behaviour that combines gravity and AABB collision
  // WARN: Deprecated
  // class CombinedGravityCollisionBehaviour : public ParticleBehaviour {
  // public:
  //   CombinedGravityCollisionBehaviour(
  //       std::shared_ptr<ParticleBehaviour> gravity,
  //       std::shared_ptr<ParticleBehaviour> collision)
  //       : gravity{std::move(gravity)}, collision{std::move(collision)} {}
  //
  //   void initialize(Particle &p) override {
  //     if (gravity)
  //       gravity->initialize(p);
  //   }
  //
  //   void update(Particle &p, float dt, const glm::mat4 &m,
  //               SimulationSpace s) override {
  //     if (gravity)
  //       gravity->update(p, dt, m, s);
  //     if (collision)
  //       collision->update(p, dt, m, s);
  //   }
  //
  //   bool isAlive(const Particle &p, const glm::mat4 &m,
  //                SimulationSpace s) const override {
  //     bool alive = true;
  //     if (gravity)
  //       alive = alive && gravity->isAlive(p, m, s);
  //     if (collision)
  //       alive = alive && collision->isAlive(p, m, s);
  //     return alive;
  //   }
  //
  //   void onDeath(Particle &p) override {
  //     if (gravity)
  //       gravity->onDeath(p);
  //   }
  //
  //   void onRespawn(Particle &p) override {
  //     if (gravity)
  //       gravity->onRespawn(p);
  //   }
  //
  // private:
  //   std::shared_ptr<ParticleBehaviour> gravity;
  //   std::shared_ptr<ParticleBehaviour> collision;
  // };

  // Create gravity behaviour
  // auto gravityBehaviour =
  //     std::make_shared<GravityBehaviour>(glm::vec3{0.0f, -9.81f, 0.0f});
  auto gravityUpdater =
      std::make_shared<GravityUpdater>(glm::vec3{0.0f, -9.81f, 0.0f});

  // Create area emitter with gravity behaviour initially
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

  // If collision targets not empty, create combined behaviour
  if (!collisionTargets.empty()) {
    // Create collision behaviour
    // auto collisionBehaviour =
    //     std::make_shared<ModelParticleAABBCollisionBehaviour>(
    //         cubeParticleSystem.get(), collisionTargets, 0.01f);
    auto collisionUpdater = std::make_shared<ModelParticleAABBCollisionUpdater>(
        cubeParticleSystem.get(), collisionTargets, 0.01f);

    // Create combined behaviour
    // auto combinedBehaviour =
    //     std::make_shared<CombinedGravityCollisionBehaviour>(gravityUpdater,
    //                                                         collisionBehaviour);
    cubeParticleSystem->addUpdater(collisionUpdater);

    // cubeEmitter->setBehaviour(combinedBehaviour);
  }

  std::println(
      std::clog,
      "Cube test system created at position ({}, {}, {}) with {} max particles",
      position.x, position.y, position.z, maxParticles);

  return cubeParticleSystem;
}
} // namespace graphics::particles
