#pragma once

#include "scene/game_object.hpp"
#include "scene/puzzle_manager.hpp"
#include "scene/game_object_behaviour.hpp"
#include <glm/glm.hpp>
#include <iostream>
#include <print>

/// Puzzle piece movement behaviour (Strategy Pattern)
/// Moves object to target position when selected, with animation
class PuzzleMovementBehaviour : public IGameObjectBehaviour {
public:
  PuzzleMovementBehaviour(PuzzleManager *puzzleManager,
                          // TODO: use relative position and rotation
                          const glm::vec3 &targetPosition,
                          const glm::vec3 &targetRotation = glm::vec3(0.0f),
                          float moveDuration = 1.0f)
      : puzzleManager(puzzleManager), targetPosition(targetPosition),
        targetRotation(targetRotation), moveDuration(moveDuration),
        isMoving(false) {
    std::cout << "PuzzleMovementBehaviour created with target position ("
              << targetPosition.x << ", " << targetPosition.y << ", "
              << targetPosition.z << ")" << std::endl;
  }

  void onSelect(GameObject *obj) override {
    if (isMoving) {
      std::cout << "Puzzle piece " << obj->getName()
                << " is already moving to target" << std::endl;
      return;
    }

    if (toggled) {
      std::cout << "Puzzle piece " << obj->getName()
                << " is already at target position" << std::endl;
      // Could move back to original position
      isMoving = true;
      toggled = false;
      obj->animateTo(originalPosition, originalRotation, moveDuration);
      return;
    }

    std::cout << "Puzzle piece " << obj->getName()
              << " moving to target position (" << targetPosition.x << ", "
              << targetPosition.y << ", " << targetPosition.z << ") over "
              << moveDuration << " seconds" << std::endl;

    // Use GameObject's built-in animation system
    obj->animateTo(targetPosition, targetRotation, moveDuration);
    isMoving = true;
    toggled = true;

    // Store original position for return
    originalPosition = obj->position;
    originalRotation = obj->rotation;
  }

  void onHover(GameObject *obj, bool enter) override {
    if (enter) {
      std::println("Hovering over puzzle piece {} ({})", obj->getName(),
                   toggled ? "toggled" : "not toggled");
    }
  }

  void onUpdate(GameObject *obj, float deltaTime) override {
    // Check if animation just completed
    if (isMoving && !obj->isAnimating()) {
      isMoving = false;
      std::cout << "Puzzle piece " << obj->getName()
                << " reached target position" << std::endl;

      if (toggled) {
        puzzleManager->checkCompletion();
      } else {
        puzzleManager->releaseOne();
      }
    }
  }

  const char *getName() const override { return "PuzzleMovementBehaviour"; }

  // Setters
  void setTargetPosition(const glm::vec3 &newTarget) {
    targetPosition = newTarget;
  }
  void setMoveDuration(float duration) { moveDuration = duration; }

private:
  PuzzleManager *puzzleManager;
  glm::vec3 targetPosition;
  glm::vec3 targetRotation;
  glm::vec3 originalPosition;
  glm::vec3 originalRotation;
  float moveDuration;
  bool isMoving;
  bool toggled;
};
