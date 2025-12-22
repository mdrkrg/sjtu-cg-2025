#pragma once

#include "scene/game_object.hpp"
#include "scene/game_object_behaviour.hpp"
#include "scene/puzzle_manager.hpp"
#include <glm/glm.hpp>
#include <print>

class HiddenCellBehaviour : public IGameObjectBehaviour {
public:
  HiddenCellBehaviour(PuzzleManager *puzzleManager,
                      const glm::vec3 &targetPosition,
                      const glm::vec3 &targetRotation = glm::vec3(0.0f),
                      float moveDuration = 1.0f)
      : puzzleManager{puzzleManager}, targetPosition{targetPosition},
        targetRotation{targetRotation}, moveDuration{moveDuration},
        obj{nullptr} {}

  ~HiddenCellBehaviour() = default;

  void onAttach(GameObject *obj) override {
    this->obj = obj;
    if (not puzzleManager) {
      std::println(std::cerr,
                   "PuzzleManager not initialized in HiddenCellBehaviour when "
                   "attaching {}",
                   obj->getName());
      return;
    }
    std::println(std::clog, "HiddenCellBehaviour attached to {}",
                 obj->getName());
    puzzleManager->registerPuzzleCallback(this, [this]() {
      std::println(std::clog, "HiddenCellBehaviour triggered!");
      if (this->obj) {
        this->obj->animateTo(targetPosition, targetRotation, moveDuration);
      }
    });
  }

  void onDetach(GameObject *) override {
    if (puzzleManager) {
      puzzleManager->unregisterPuzzleCallback(this);
    }
    this->obj = nullptr;
  }

  const char *getName() const override { return "HiddenCellBehaviour"; }

private:
  PuzzleManager *puzzleManager;
  glm::vec3 targetPosition;
  glm::vec3 targetRotation;
  float moveDuration;
  GameObject *obj;
};
