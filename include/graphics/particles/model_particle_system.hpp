#pragma once

#include "base/emitter.hpp"
#include "particle_system.hpp"
#include "model_particle.hpp"
#include <graphics/model.hpp>
#include <graphics/material.hpp>
#include <graphics/shader.h>
#include <memory>
#include <vector>

namespace graphics::particles {

/// Model-based particle system for instanced rendering of 3D models
/// Extends ParticleSystem<ModelParticle> to replace GL_POINTS rendering with
/// instanced model rendering
class ModelParticleSystem : public ParticleSystem<ModelParticle> {
public:
  /// Constructor
  /// @param shader The particle shader used for the system
  /// @param emitter Particle emitter
  /// @param modelWithMaterials Shared model with materials for all particles
  /// @param space Simulation space
  /// @param customTransform Custom transform for CUSTOM simulation space
  ModelParticleSystem(std::shared_ptr<Shader> shader,
                      std::shared_ptr<Emitter> emitter,
                      ModelWithMaterials modelWithMaterials,
                      SimulationSpace space = SimulationSpace::WORLD,
                      GameObject *customTransform = nullptr);

  ~ModelParticleSystem() override;

  // ParticleSystem methods (virtual in base)
  void update(float deltaTime) override;
  void render(const glm::mat4 &view, const glm::mat4 &projection) override;
  void emitParticle() override;
  void reset() override;
  void init() override;

  /// Get world AABB for the particle system combined. Good for debugging
  scene::AABB getWorldAABB() const;

  /// Get individual world AABBs for all active particles
  std::vector<scene::AABB> getParticleWorldAABBs() const;

  bool isVisible() const;

  const scene::AABB &getParticleLocalAABB() const { return particleLocalAABB; }

  void setMaxParticles(size_t max);

  // Instanced rendering control
  void setupInstancedRendering();
  void cleanupInstancedRendering();
  void updateInstanceData();

  friend class ArrowLauncher;

private:
  // Shared resources (shared by all particles)
  ModelWithMaterials particleModel;
  scene::AABB particleLocalAABB;

  // Instanced rendering
  unsigned int instanceVBO{0};
  std::vector<glm::mat4> instanceMatrices;
  bool instanceBufferInitialized{false};

  // Helper methods
  void computeModelAABB();
  void setupInstanceBuffer();
  void updateParticlePool();

  // ParticleSystem buffer methods (hide point sprite rendering)
  void setupBuffers() override {}
  void loadTexture() override {}
  void updateBuffers() override {}
  void cleanup() override;
};

} // namespace graphics::particles
