#include "graphics/particles/model_particle_system.hpp"
#include "graphics/rendering.hpp"
#include "scene/aabb.hpp"

namespace graphics::particles {

ModelParticleSystem::ModelParticleSystem(std::shared_ptr<Shader> shader,
                                         std::shared_ptr<Emitter> emitter,
                                         ModelWithMaterials modelWithMaterials,
                                         SimulationSpace space,
                                         GameObject *customTransform)
    : ParticleSystem{shader, emitter, space, customTransform},
      particleModel{std::move(modelWithMaterials)} {

  // Compute local AABB from model
  computeModelAABB();

  // Initialize particle pool with ModelParticles
  updateParticlePool();
}

ModelParticleSystem::~ModelParticleSystem() { cleanupInstancedRendering(); }

void ModelParticleSystem::init() {
  // Override ParticleSystem::init(), no point sprite buffers
  setupInstancedRendering();
}

void ModelParticleSystem::update(float deltaTime) {
  // Call base update for emission and lifecycle management
  ParticleSystem<ModelParticle>::update(deltaTime);

  // Update ModelParticle-specific state
  for (auto &particle : activeParticles) {
    particle.update(deltaTime);
  }

  // Update instance data for rendering
  updateInstanceData();
}

void ModelParticleSystem::render(const glm::mat4 &view,
                                 const glm::mat4 &projection) {
  // Override to use instanced rendering instead of GL_POINTS
  // Instanced rendering implementation
  if (not particleModel.model or not isVisible()) {
    return;
  }
  shader->use();

  // Set transformation uniforms
  shader->setMat4("view", view);
  shader->setMat4("projection", projection);

  const auto &model = *particleModel.model;
  const auto &materials = particleModel.materials;

  const size_t meshCount = model.meshes.size();
  const size_t materialCount = materials.size();

  for (size_t i = 0; i < meshCount and i < materialCount; ++i) {
    const auto &meshMat = materials[i];
    const auto &mesh = model.meshes[i];
    graphics::renderMeshInstanced(mesh, meshMat, *shader,
                                  instanceMatrices.size());
  }
}

void ModelParticleSystem::emitParticle() {
  if (activeParticles.size() >= maxParticles) {
    return;
  }

  ModelParticle particle{};
  if (!particlePool.empty()) {
    particle = particlePool.front();
    particlePool.pop();
  } else {
    // Create new particle
    particle = ModelParticle{};
  }

  // Initialize particle using emitter
  if (emitter) {
    emitter->emit(particle);
  }

  // Add to active particles
  activeParticles.push_back(particle);
}

void ModelParticleSystem::reset() {
  // Move all active particles back to pool
  for (auto &particle : activeParticles) {
    particlePool.push(particle);
  }
  activeParticles.clear();
  instanceMatrices.clear();
}

void ModelParticleSystem::setMaxParticles(size_t max) {
  maxParticles = max;
  updateParticlePool();
}

scene::AABB ModelParticleSystem::getWorldAABB() const {
  scene::AABB combined{};
  const glm::mat4 systemModel = getModelMatrix();

  for (const auto &particle : activeParticles) {
    // Transform particle local AABB to world space
    glm::mat4 particleTransform = systemModel * particle.getTransformMatrix();
    const auto &particleAABB = particleLocalAABB.transform(particleTransform);

    combined = combined.merge(particleAABB);
  }

  return combined;
}

std::vector<scene::AABB> ModelParticleSystem::getParticleWorldAABBs() const {
  std::vector<scene::AABB> particleAABBs;
  const glm::mat4 systemModel = getModelMatrix();

  for (const auto &particle : activeParticles) {
    // Transform particle local AABB to world space
    glm::mat4 particleTransform = systemModel * particle.getTransformMatrix();
    const auto &particleAABB = particleLocalAABB.transform(particleTransform);

    particleAABBs.push_back(particleAABB);
  }

  return particleAABBs;
}

bool ModelParticleSystem::isVisible() const { return !activeParticles.empty(); }

void ModelParticleSystem::setupInstancedRendering() {
  if (instanceBufferInitialized or not particleModel.model) {
    return;
  }

  // Create the instance VBO and set up instance attributes
  // for all meshes in the model
  setupInstanceBuffer();

  instanceBufferInitialized = true;
}

void ModelParticleSystem::cleanupInstancedRendering() {
  if (not instanceBufferInitialized) {
    return;
  }

  if (instanceVBO != 0) {
    glDeleteBuffers(1, &instanceVBO);
    instanceVBO = 0;
  }
  instanceMatrices.clear();
  instanceBufferInitialized = false;
}

void ModelParticleSystem::updateInstanceData() {
  instanceMatrices.clear();
  instanceMatrices.reserve(activeParticles.size());

  const glm::mat4 systemModel = getModelMatrix();

  for (const auto &particle : activeParticles) {
    if (particle.alive()) {
      // Combine system transform with particle transform
      glm::mat4 transform = systemModel * particle.getTransformMatrix();
      instanceMatrices.push_back(transform);
    }
  }

  // Update instance buffer if we have instances
  if (not instanceMatrices.empty() and instanceVBO != 0) {
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, instanceMatrices.size() * sizeof(glm::mat4),
                 instanceMatrices.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }
}

void ModelParticleSystem::computeModelAABB() {
  if (!particleModel.model || particleModel.model->meshes.empty()) {
    // Default small cube AABB
    particleLocalAABB = scene::AABB{glm::vec3{-0.1f}, glm::vec3{0.1f}};
    return;
  }

  glm::vec3 min{std::numeric_limits<float>::max()};
  glm::vec3 max{std::numeric_limits<float>::lowest()};

  // Compute AABB from all meshes
  for (const auto &mesh : particleModel.model->meshes) {
    for (const auto &vertex : mesh.vertices) {
      min = glm::min(min, vertex.Position);
      max = glm::max(max, vertex.Position);
    }
  }

  if (min.x == std::numeric_limits<float>::max()) {
    // No vertices found
    particleLocalAABB = scene::AABB{glm::vec3{-0.1f}, glm::vec3{0.1f}};
  } else {
    particleLocalAABB = scene::AABB{min, max};
  }
}

void ModelParticleSystem::setupInstanceBuffer() {
  if (not particleModel.model or particleModel.model->meshes.empty()) {
    return;
  }

  // Create instance buffer
  glGenBuffers(1, &instanceVBO);
  glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);

  // Initial empty buffer
  glBufferData(GL_ARRAY_BUFFER, maxParticles * sizeof(glm::mat4), nullptr,
               GL_DYNAMIC_DRAW);

  // For each mesh VAO, set up instance attribute
  for (const auto &mesh : particleModel.model->meshes) {
    glBindVertexArray(mesh.VAO);

    // mat4 takes 4 attribute locations (3-6)
    size_t baseLocation = 3; // After position(0), normal(1), texcoord(2)
    for (int i = 0; i < 4; ++i) {
      glEnableVertexAttribArray(baseLocation + i);
      glVertexAttribPointer(baseLocation + i, 4, GL_FLOAT, GL_FALSE,
                            sizeof(glm::mat4), (void *)(i * sizeof(glm::vec4)));
      glVertexAttribDivisor(baseLocation + i, 1); // Update once per instance
    }

    glBindVertexArray(0);
  }

  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void ModelParticleSystem::updateParticlePool() {
  std::queue<ModelParticle> newPool{};

  // Move existing ModelParticles to new pool
  while (!particlePool.empty()) {
    newPool.push(std::move(particlePool.front()));
    particlePool.pop();
  }

  // Add new ModelParticles if needed
  while (newPool.size() < maxParticles) {
    newPool.push(ModelParticle{});
  }

  particlePool = std::move(newPool);
}

void ModelParticleSystem::cleanup() {
  // No point sprite, no ParticleSystem::cleanup()
  cleanupInstancedRendering();
}

} // namespace graphics::particles
