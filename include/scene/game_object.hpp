#pragma once

#include <epoxy/gl.h>
#include <epoxy/glx.h>
#include <functional>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <vector>

#include "graphics/model.hpp"
#include "graphics/model_factory.hpp"
#include "graphics/shader.h"
#include "scene/aabb.hpp"
#include "scene/game_object_behaviour.hpp"
#include "graphics/material.hpp"
#include "graphics/rendering.hpp"
#include "graphics/mesh.hpp"

class GameObject {
public:
  GameObject(std::shared_ptr<Model> model, std::shared_ptr<Shader> shader,
             const Material &material, const std::string &name = "");
  GameObject(ModelWithMaterials &&modelMaterials,
             std::shared_ptr<Shader> shader, const std::string &name = "");
  ~GameObject();

  /// Factory method to create a model from model file
  static std::unique_ptr<GameObject>
  createFromModelFile(const std::string &path, std::shared_ptr<Shader> shader,
                      const std::string &name = "");

  /// Whether this GameObject is the raycast target
  bool interactable = true;
  std::function<void(GameObject *)> onselect = [](GameObject *) {};
  std::function<void(GameObject *)> onhover = [](GameObject *) {};

  // Rendering
  void render(const glm::mat4 &projection, const glm::mat4 &view);

  // Update animation
  void update(float deltaTime);

  // Transform
  glm::vec3 position{0.0f};
  glm::vec3 rotation{0.0f}; // Euler angles in degrees (X, Y, Z)
  glm::vec3 scale{1.0f};

  glm::mat4 getModelMatrix() const;

  // Interaction
  const std::string &getName() const { return name; }

  bool isSelected() const { return selected; }

  void setSelected(bool h) {
    selected = h;
    if (h) {
      onselect(this);
      // Call behaviours
      for (auto &behaviour : behaviours) {
        behaviour->onSelect(this);
      }
    }
  }

  void toggleSelected() {
    selected = not selected;
    if (selected) {
      onselect(this);
      // Call behaviours
      for (auto &behaviour : behaviours) {
        behaviour->onSelect(this);
      }
    }
  }

  bool isHovered() const { return hovered; }

  void setHovered(bool h) {
    hovered = h;
    if (h) {
      onhover(this);
    }
    // Call behaviours (both enter and exit)
    for (auto &behaviour : behaviours) {
      behaviour->onHover(this, h);
    }
  }

  void toggleHovered() {
    hovered = not hovered;
    if (hovered) {
      onhover(this);
    }
    // Call behaviours
    for (auto &behaviour : behaviours) {
      behaviour->onHover(this, hovered);
    }
  }

  // Get world-space AABB (transformed from local AABB)
  scene::AABB getWorldAABB() const;

  /// Check if a point collides with this GameObject using mesh collision
  /// @param worldPoint Point in world space to test
  /// @return true if point is inside any mesh of this GameObject
  bool checkPointCollision(const glm::vec3 &worldPoint) const;

  /// Perform ray casting against this GameObject
  /// @param ray The ray in world space (normalized)
  /// @return Optional ray hit information if ray intersects any mesh
  std::optional<math::RayHit> rayCast(const math::Ray &ray) const;

  /// Get simplified collision meshes for this GameObject
  /// Generates simplified version if not already created
  /// @return Reference to collision mesh vector
  const std::vector<Mesh> &getCollisionMeshes() const;

  // Animation
  void animateTo(const glm::vec3 &targetPosition,
                 const glm::vec3 &targetRotation, float duration);
  bool isAnimating() const { return animation.active; }
  void stopAnimation() { animation.active = false; }

  Model *getModel() { return model.get(); }
  const Model *getModel() const { return model.get(); }

  Material &getMeshMaterial(size_t index) {
    assert(index < meshMaterials.size());
    return meshMaterials[index];
  }

  const Material &getMeshMaterial(size_t index) const {
    assert(index < meshMaterials.size());
    return meshMaterials[index];
  }

  void setMeshMaterial(size_t index, const Material &mat) {
    assert(index < meshMaterials.size());
    meshMaterials[index] = mat;
  }

  size_t getMeshMaterialCount() const { return meshMaterials.size(); }

  Shader *getShader() const { return shader.lock().get(); }
  void setShader(std::shared_ptr<Shader> shader) { this->shader = shader; }

  // Behaviour system (Strategy Pattern)
  void addBehaviour(std::unique_ptr<IGameObjectBehaviour> behaviour) {
    behaviour->onAttach(this);
    behaviours.push_back(std::move(behaviour));
  }

  void updateBehaviours(float deltaTime) {
    for (auto &behaviour : behaviours) {
      behaviour->onUpdate(this, deltaTime);
    }
  }

  void preRenderBehaviours() {
    for (auto &behaviour : behaviours) {
      behaviour->onPreRender(this);
    }
  }

  void postRenderBehaviours() {
    for (auto &behaviour : behaviours) {
      behaviour->onPostRender(this);
    }
  }

  // Get behaviours (for debugging or specific behaviour access)
  const std::vector<std::unique_ptr<IGameObjectBehaviour>> &
  getBehaviours() const {
    return behaviours;
  }

private:
  std::shared_ptr<Model> model;
  // Pointer to shader (owned by GraphicsRenderer)
  std::weak_ptr<Shader> shader;
  // One material per mesh
  std::vector<Material> meshMaterials;
  std::string name;
  bool selected = false;
  bool hovered = false;

  // Behaviour system
  std::vector<std::unique_ptr<IGameObjectBehaviour>> behaviours;

  // Local-space AABB (computed from model vertices)
  scene::AABB localAABB;

  // Animation state
  struct Animation {
    glm::vec3 startPosition;
    glm::vec3 targetPosition;
    glm::vec3 startRotation;
    glm::vec3 targetRotation;
    float time = 0.0f;
    float duration = 0.0f;
    bool active = false;
  } animation;

  // Compute local AABB from model vertices
  void computeLocalAABB();

  /// Render all meshes
  void renderMeshes(const Shader &shader) {
    const size_t meshCount = model->meshes.size();
    const size_t materialCount = meshMaterials.size();
    checkMeshMaterialMismatch();

    // Render each mesh with its material
    for (size_t i = 0; i < meshCount and i < materialCount; ++i) {
      const auto &meshMat = meshMaterials[i];
      const auto &mesh = model->meshes[i];
      graphics::renderMesh(mesh, meshMat, shader);
    }
  }

  // Helper for linear interpolation
  static glm::vec3 lerp(const glm::vec3 &a, const glm::vec3 &b, float t);

  void checkMeshMaterialMismatch(
      std::function<void(size_t meshCount, size_t materialCount)> onMismatch =
          [](size_t, size_t) {}) {
    const size_t meshCount = model->meshes.size();
    const size_t materialCount = meshMaterials.size();
    if (meshCount != materialCount) {
      std::println(std::clog,
                   "Warning: Mesh count ({}) doesn't match material count ({}) "
                   "for GameObject {}",
                   meshCount, materialCount, name);
      onMismatch(meshCount, materialCount);
    }
  }
};

inline GameObject::~GameObject() {
  for (const auto &behaviour : behaviours) {
    behaviour->onDetach(this);
  }
}

inline GameObject::GameObject(std::shared_ptr<Model> model,
                              std::shared_ptr<Shader> shader,
                              const Material &material, const std::string &name)
    : model(std::move(model)), shader(shader), name(name) {
  // Initialize mesh materials vector with one material per mesh
  if (this->model) {
    size_t meshCount = this->model->meshes.size();
    meshMaterials.reserve(meshCount);
    for (size_t i = 0; i < meshCount; ++i) {
      meshMaterials.push_back(material);
    }
  }
  computeLocalAABB();
}

inline GameObject::GameObject(ModelWithMaterials &&modelMaterials,
                              std::shared_ptr<Shader> shader,
                              const std::string &name)
    : model(std::move(modelMaterials.model)), shader(shader),
      meshMaterials(std::move(modelMaterials.materials)), name(name) {

  if (not model) {
    computeLocalAABB();
    return;
  }

  checkMeshMaterialMismatch([this](size_t meshCount, size_t materialCount) {
    std::println(std::clog, "Using default material for missing meshes.");
    if (materialCount < meshCount) {
      Material defaultMat{};
      for (size_t i = materialCount; i < meshCount; ++i) {
        meshMaterials.push_back(defaultMat);
      }
    }
  });
  computeLocalAABB();
}

inline std::unique_ptr<GameObject>
GameObject::createFromModelFile(const std::string &path,
                                std::shared_ptr<Shader> shader,
                                const std::string &name) {
  auto model = ModelFactory::loadModel(path);
  return std::make_unique<GameObject>(std::move(model), shader, name);
}
inline void GameObject::computeLocalAABB() {
  if (not model or model->meshes.empty()) {
    localAABB =
        scene::AABB(glm::vec3(-0.1f), glm::vec3(0.1f)); // Default small cube
    return;
  }

  glm::vec3 min(std::numeric_limits<float>::max());
  glm::vec3 max(std::numeric_limits<float>::lowest());

  for (const auto &mesh : model->meshes) {
    for (const auto &vertex : mesh.vertices) {
      min = glm::min(min, vertex.Position);
      max = glm::max(max, vertex.Position);
    }
  }

  // If no vertices found (shouldn't happen), use default
  if (min.x == std::numeric_limits<float>::max()) {
    min = glm::vec3(-0.1f);
    max = glm::vec3(0.1f);
  }

  localAABB = scene::AABB(min, max);
}

inline void GameObject::render(const glm::mat4 &projection,
                               const glm::mat4 &view) {
  const auto shader = this->shader.lock();
  if (not model or not shader)
    return;

  // Call pre-render behaviours (can modify shader/material)
  preRenderBehaviours();

  shader->use();

  // Set transformation uniforms
  shader->setMat4("projection", projection);
  shader->setMat4("view", view);
  shader->setMat4("model", getModelMatrix());

  renderMeshes(*shader.get());

  // Call post-render behaviours
  postRenderBehaviours();
}

inline void GameObject::update(float deltaTime) {
  // Update behaviours first
  updateBehaviours(deltaTime);

  // Then update animation if active
  if (!animation.active)
    return;

  animation.time += deltaTime;
  float t = glm::clamp(animation.time / animation.duration, 0.0f, 1.0f);

  // Linear interpolation
  position = lerp(animation.startPosition, animation.targetPosition, t);
  rotation = lerp(animation.startRotation, animation.targetRotation, t);

  if (t >= 1.0f) {
    animation.active = false;
  }
}

inline glm::mat4 GameObject::getModelMatrix() const {
  glm::mat4 modelMatrix(1.0f);
  modelMatrix = glm::translate(modelMatrix, position);
  modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.x),
                            glm::vec3(1.0f, 0.0f, 0.0f));
  modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.y),
                            glm::vec3(0.0f, 1.0f, 0.0f));
  modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.z),
                            glm::vec3(0.0f, 0.0f, 1.0f));
  modelMatrix = glm::scale(modelMatrix, scale);
  return modelMatrix;
}

inline scene::AABB GameObject::getWorldAABB() const {
  return localAABB.transform(getModelMatrix());
}

inline void GameObject::animateTo(const glm::vec3 &targetPosition,
                                  const glm::vec3 &targetRotation,
                                  float duration) {
  if (duration <= 0.0f) {
    position = targetPosition;
    rotation = targetRotation;
    return;
  }

  animation.startPosition = position;
  animation.targetPosition = targetPosition;
  animation.startRotation = rotation;
  animation.targetRotation = targetRotation;
  animation.time = 0.0f;
  animation.duration = duration;
  animation.active = true;
}

inline glm::vec3 GameObject::lerp(const glm::vec3 &a, const glm::vec3 &b,
                                  float t) {
  return a * (1.0f - t) + b * t;
}

inline bool GameObject::checkPointCollision(const glm::vec3 &worldPoint) const {
  if (not model or model->meshes.empty()) {
    return false;
  }

  glm::mat4 modelMatrix = getModelMatrix();

  // Check each mesh in the model
  for (const auto &collisionMesh : getCollisionMeshes()) {
    if (collisionMesh.containsPoint(worldPoint, modelMatrix)) {
      return true;
    }
  }

  return false;
}

inline std::optional<math::RayHit>
GameObject::rayCast(const math::Ray &ray) const {

  if (not model or model->meshes.empty()) {
    return std::nullopt;
  }

  glm::mat4 modelMatrix = getModelMatrix();
  std::optional<math::RayHit> closestHit = std::nullopt;
  float closestDistance = std::numeric_limits<float>::max();

  // Check each mesh in the model
  for (const auto &collisionMesh : getCollisionMeshes()) {
    const auto hit = collisionMesh.rayIntersection(ray, modelMatrix);

    if (hit and hit->distance < closestDistance) {
      closestHit = hit;
      closestDistance = hit->distance;
    }
  }

  return closestHit;
}

inline const std::vector<Mesh> &GameObject::getCollisionMeshes() const {
  if (not model) {
    // TODO: Better fallback
    throw std::runtime_error("Expected model to exist");
  }

  // TODO: Return the simplified mesh from the model meshes
  return model->meshes;
}
