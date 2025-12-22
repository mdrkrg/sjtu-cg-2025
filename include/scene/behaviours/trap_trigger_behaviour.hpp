#pragma once

#include "scene/game_object.hpp"
#include "scene/trap_manager.hpp"
#include "scene/game_object_behaviour.hpp"
#include <glm/glm.hpp>
#include <iostream>

class TrapTriggerBehaviour : public IGameObjectBehaviour {
public:
  TrapTriggerBehaviour(TrapManager *trapManager) : trapManager{trapManager} {
    std::println(std::clog, "TrapTriggerBehaviour created");
  }

  void onSelect(GameObject *) override { trapManager->trigger(); }

  virtual const char *getName() const override {
    return "TrapTriggerBehaviour";
  };

private:
  TrapManager *trapManager;
};
