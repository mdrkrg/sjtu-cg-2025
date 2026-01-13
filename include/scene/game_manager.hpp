#pragma once

#include "game_object.hpp"
#include "puzzle_manager.hpp"
#include "trap_manager.hpp"
#include <glm/glm.hpp>
#include <memory>
#include <optional>
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
  GameObject *handleRayCast(const math::Ray &ray);

  /// Get all objects for rendering
  const std::vector<GameObject *> &getObjects() const { return objectPointers; }

  /// Get the first occurance of a GameObject by name
  std::optional<GameObject *> getObject(const std::string &name) const {
    for (const auto obj : objectPointers) {
      if (obj->getName() == name) {
        return obj;
      }
    }

    return std::nullopt;
  }

  PuzzleManager *getPuzzleManager() const { return puzzleManager.get(); }

  TrapManager *getTrapManager() const { return trapManager.get(); }

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
  // managers are declared before objects,
  // so that they can be unregistered safely in object deconstruction

  std::unique_ptr<PuzzleManager> puzzleManager;
  std::unique_ptr<TrapManager> trapManager;

  // TODO: Use a map of names to objects?
  std::vector<std::unique_ptr<GameObject>> objects;
  std::vector<GameObject *> objectPointers; // For fast iteration
  GameObject *selectedObject = nullptr;
  GameObject *hoveredObject = nullptr;

  std::function<void(GameObject *)> onObjectSelected;
  std::function<void(GameObject *, bool)> onObjectHovered;

  void updateObjectPointers();
};
