#pragma once

#include "scene/game_object.hpp"
#include "scene/puzzle_manager.hpp"
#include "scene/game_object_behaviour.hpp"
#include <glm/glm.hpp>
#include <iostream>

/// Puzzle piece movement behaviour (Strategy Pattern)
/// Moves object to target position when selected, with animation
class PuzzleMovementBehaviour : public IGameObjectBehaviour {
public:
  PuzzleMovementBehaviour(PuzzleManager *puzzleManager,
                          const glm::vec3 &targetPosition,
                          const glm::vec3 &targetRotation = glm::vec3(0.0f),
                          float moveDuration = 1.0f)
      : puzzleManager(puzzleManager), targetPosition(targetPosition),
        targetRotation(targetRotation), moveDuration(moveDuration),
        isAtTarget(false), isMoving(false) {
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

    if (isAtTarget) {
      std::cout << "Puzzle piece " << obj->getName()
                << " is already at target position" << std::endl;
      // Could move back to original position
      // glm::vec3 originalPos = ...; // Would need to store original position
      // obj->animateTo(originalPos, originalRot, moveDuration);
      // isAtTarget = false;
      return;
    }

    std::cout << "Puzzle piece " << obj->getName()
              << " moving to target position (" << targetPosition.x << ", "
              << targetPosition.y << ", " << targetPosition.z << ") over "
              << moveDuration << " seconds" << std::endl;

    // Use GameObject's built-in animation system
    obj->animateTo(targetPosition, targetRotation, moveDuration);
    isMoving = true;
    isAtTarget = false;

    // Store original position for potential return
    originalPosition = obj->position;
    originalRotation = obj->rotation;
  }

  void onHover(GameObject *obj, bool enter) override {
    if (enter) {
      std::cout << "Hovering over puzzle piece " << obj->getName() << " ("
                << (isAtTarget ? "at target" : "not at target") << ")"
                << std::endl;
    }
  }

  void onUpdate(GameObject *obj, float deltaTime) override {
    // Check if animation just completed
    if (isMoving && !obj->isAnimating()) {
      isMoving = false;
      isAtTarget = true;
      std::cout << "Puzzle piece " << obj->getName()
                << " reached target position" << std::endl;

      // Could trigger puzzle completion check here
      puzzleManager->checkCompletion();
    }

    // Could add hover/selection highlight effect here
    // For example: slight bobbing animation when hovered
  }

  void onPreRender(GameObject *obj) override {
    // Could change material color based on state
    // if (isAtTarget) -> green tint
    // if (isMoving) -> yellow tint
    // else -> default color

    // In full implementation:
    // obj->material.diffuse = stateColor;
  }

  void onPostRender(GameObject *obj) override {
    // Reset any temporary modifications
  }

  const char *getName() const override { return "PuzzleMovementBehaviour"; }

  // Getters
  bool getIsAtTarget() const { return isAtTarget; }
  bool getIsMoving() const { return isMoving; }
  glm::vec3 getTargetPosition() const { return targetPosition; }

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
  bool isAtTarget;
  bool isMoving;
};
