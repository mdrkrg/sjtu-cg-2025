#pragma once

#include "scene/game_object.hpp"
#include "scene/trap_manager.hpp"
#include "scene/game_object_behaviour.hpp"
#include <glm/glm.hpp>
#include <iostream>
#include <print>

class TrapBehaviour : public IGameObjectBehaviour {
public:
  TrapBehaviour(TrapManager *trapManager, const glm::vec3 &targetPosition,
                const glm::vec3 &targetRotation = glm::vec3(0.0f),
                float moveDuration = 1.0f)
      : trapManager{trapManager}, targetPosition{targetPosition},
        targetRotation{targetRotation}, moveDuration{moveDuration},
        obj{nullptr} {
    std::println(std::clog, "TrapBehaviour created");
  }

  void onAttach(GameObject *obj) override {
    this->obj = obj;
    if (not trapManager) {
      return;
    }
    trapManager->registerTriggerCallback(this, [this] {
      if (this->obj) {
        this->obj->animateTo(targetPosition, targetRotation, moveDuration);
        std::println("Trap triggerred!!");
      }
    });
  }

  void onDetach(GameObject *) override {
    if (trapManager) {
      trapManager->unregisterTriggerCallback(this);
    }
    this->obj = nullptr;
  }

  virtual const char *getName() const override { return "TrapBehaviour"; };

private:
  TrapManager *trapManager;
  glm::vec3 targetPosition;
  glm::vec3 targetRotation;
  float moveDuration;
  GameObject *obj;
};
