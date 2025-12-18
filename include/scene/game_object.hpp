#pragma once

#include <epoxy/gl.h>
#include <epoxy/glx.h>
#include <functional>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <vector>

#include "graphics/model.hpp"
#include "graphics/shader.h"
#include "scene/aabb.hpp"
#include "scene/game_object_behaviour.hpp"

class GameObject {
public:
  GameObject(std::unique_ptr<Model> model, bool interactive = false,
             const std::string &name = "");
  ~GameObject() = default;

  std::function<void(GameObject *)> onselect = [](GameObject *) {};
  std::function<void(GameObject *)> onhover = [](GameObject *) {};

  // Rendering
  void render(Shader &shader);

  // Update animation
  void update(float deltaTime);

  // Transform
  glm::vec3 position{0.0f};
  glm::vec3 rotation{0.0f}; // Euler angles in degrees (X, Y, Z)
  glm::vec3 scale{1.0f};

  glm::mat4 getModelMatrix() const;

  // Interaction
  bool isInteractive() const { return interactive; }
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

  // Animation
  void animateTo(const glm::vec3 &targetPosition,
                 const glm::vec3 &targetRotation, float duration);
  bool isAnimating() const { return animation.active; }
  void stopAnimation() { animation.active = false; }

  // Model access (for material modification, e.g., lamp emission)
  Model *getModel() { return model.get(); }
  const Model *getModel() const { return model.get(); }

  // Behaviour system (Strategy Pattern)
  void addBehaviour(std::unique_ptr<IGameObjectBehaviour> behaviour) {
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
  std::unique_ptr<Model> model;
  bool interactive;
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

  // Helper for linear interpolation
  static glm::vec3 lerp(const glm::vec3 &a, const glm::vec3 &b, float t);
};

// GameObject implementation
inline GameObject::GameObject(std::unique_ptr<Model> model, bool interactive,
                              const std::string &name)
    : model(std::move(model)), interactive(interactive), name(name) {
  computeLocalAABB();
}

inline void GameObject::computeLocalAABB() {
  if (!model || model->meshes.empty()) {
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

inline void GameObject::render(Shader &shader) {
  if (!model)
    return;

  // Call pre-render behaviours (can modify shader/material)
  preRenderBehaviours();

  // Set model matrix
  glm::mat4 modelMatrix = getModelMatrix();
  shader.setMat4("model", modelMatrix);

  // Render model
  model->Draw(shader);

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
