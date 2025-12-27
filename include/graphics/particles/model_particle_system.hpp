#pragma once

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
  /// @param emitter Particle emitter
  /// @param space Simulation space
  /// @param customTransform Custom transform for CUSTOM simulation space
  /// @param particleModel Shared model for all particles
  /// @param particleMaterial Shared material for all particles
  ModelParticleSystem(std::shared_ptr<ParticleEmitter> emitter,
                      SimulationSpace space = SimulationSpace::WORLD,
                      GameObject *customTransform = nullptr,
                      std::shared_ptr<Model> particleModel = nullptr,
                      std::shared_ptr<Material> particleMaterial = nullptr);

  ~ModelParticleSystem() override;

  // ParticleSystem methods (virtual in base)
  void update(float deltaTime) override;
  void render(Shader &shader, const glm::mat4 &view,
              const glm::mat4 &projection) override;
  void emitParticle() override;
  void reset() override;
  void init() override;

  /// Get world AABB for the particle system combined. Good for debugging
  scene::AABB getWorldAABB() const;

  bool isVisible() const;

  // Resource access
  std::shared_ptr<Model> getParticleModel() const { return particleModel; }
  std::shared_ptr<Material> getParticleMaterial() const {
    return particleMaterial;
  }
  const scene::AABB &getParticleLocalAABB() const { return particleLocalAABB; }

  void setMaxParticles(size_t max);

  // Instanced rendering control
  void setupInstancedRendering();
  void cleanupInstancedRendering();
  void updateInstanceData();

  friend class ArrowLauncher;

private:
  // Shared resources (shared by all particles)
  std::shared_ptr<Model> particleModel;
  std::shared_ptr<Material> particleMaterial;
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
