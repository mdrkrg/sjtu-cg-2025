#pragma once

#include <glm/glm.hpp>

// Forward declaration
class GameObject;

/// Interface for GameObject behaviours (Strategy Pattern)
/// Similar to ParticleBehaviour in particle system
class IGameObjectBehaviour {
public:
  virtual ~IGameObjectBehaviour() = default;

  /// Called when object is selected (clicked)
  virtual void onSelect(GameObject *obj) { (void)obj; };

  /// Called when object is hovered over
  virtual void onHover(GameObject *obj, bool enter) {
    (void)obj;
    (void)enter;
  };

  /// Called every frame for behaviour updates
  virtual void onUpdate(GameObject *obj, float deltaTime) {
    (void)obj;
    (void)deltaTime;
  };

  /// Called before rendering for material/shader modifications
  virtual void onPreRender(GameObject *obj) { (void)obj; };

  /// Called after rendering
  virtual void onPostRender(GameObject *obj) { (void)obj; };

  /// Get behaviour name for debugging
  virtual const char *getName() const = 0;
};
