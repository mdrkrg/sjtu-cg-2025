#pragma once

#include "game_object.hpp"
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <functional>

class GameManager {
public:
  GameManager();
  ~GameManager() = default;

  /// Add a GameObject to the manager
  GameObject *addObject(std::unique_ptr<GameObject> obj);

  /// Update all GameObjects (animations, behaviours)
  void update(float deltaTime);

  /// Handle ray casting for object selection
  /// Returns selected GameObject or nullptr
  GameObject *handleRayCast(const glm::vec3 &rayOrigin,
                            const glm::vec3 &rayDir);

  /// Get all objects for rendering
  const std::vector<GameObject *> &getObjects() const { return objectPointers; }

  /// Get selected object
  GameObject *getSelectedObject() const { return selectedObject; }

  /// Clear all objects
  void clear();

  /// Find object by name
  GameObject *findObjectByName(const std::string &name) const;

  /// Set callback for object selection changes
  void setOnObjectSelected(std::function<void(GameObject *)> callback) {
    onObjectSelected = callback;
  }

  /// Set callback for object hover changes
  void setOnObjectHovered(std::function<void(GameObject *, bool)> callback) {
    onObjectHovered = callback;
  }

private:
  std::vector<std::unique_ptr<GameObject>> objects;
  std::vector<GameObject *> objectPointers; // For fast iteration
  GameObject *selectedObject = nullptr;
  GameObject *hoveredObject = nullptr;

  std::function<void(GameObject *)> onObjectSelected;
  std::function<void(GameObject *, bool)> onObjectHovered;

  void updateObjectPointers();
};
