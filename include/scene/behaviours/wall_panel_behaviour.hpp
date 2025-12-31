#pragma once

#include "scene/game_object.hpp"
#include "scene/game_object_behaviour.hpp"
#include "scene/trap_manager.hpp"
#include <glm/glm.hpp>
#include <print>
#include <functional>

class WallPanelBehaviour : public IGameObjectBehaviour {
public:
  enum class State {
    Locked,    // Cannot interact (not revealed yet)
    Unlocked,  // Can interact, appears as wall
    Revealing, // Animation in progress
    Revealed   // Compartment visible, arrows shot
  };

  WallPanelBehaviour(TrapManager *trapManager, GameObject *cavityObj,
                     std::function<void()> arrowCallback = nullptr,
                     const glm::vec3 &cavityScale = glm::vec3(0.15f, 0.15f,
                                                              0.1f),
                     float revealDuration = 0.8f)
      : trapManager{trapManager}, cavityObj{cavityObj},
        arrowCallback{arrowCallback}, cavityScale{cavityScale},
        revealDuration{revealDuration}, state{State::Locked}, obj{nullptr},
        timeAccumulator{0.0f} {}

  ~WallPanelBehaviour() = default;

  void onAttach(GameObject *obj) override {
    // obj should not be interactable initially
    this->obj = obj;
    if (not trapManager) {
      std::println(std::cerr,
                   "TrapManager not initialized in WallPanelBehaviour when "
                   "attaching {}",
                   obj->getName());
      return;
    }

    std::println(std::clog, "WallPanelBehaviour attached to {}",
                 obj->getName());

    // Register callback for when bookcase moves (trap triggered)
    trapManager->registerTriggerCallback(this, [this] {
      std::println(std::clog,
                   "WallPanelBehaviour: Bookcase moved, panel unlocked!");
      if (this->obj) {
        this->obj->interactable = true; // Now can be clicked
        state = State::Unlocked;
      }
    });
  }

  void onDetach(GameObject *) override {
    if (trapManager) {
      trapManager->unregisterTriggerCallback(this);
    }
    this->obj = nullptr;
  }

  void onSelect(GameObject *obj) override {
    if (state != State::Unlocked) {
      std::println(std::clog, "Wall panel {} not unlocked (state: {})",
                   obj->getName(), static_cast<int>(state));
      return;
    }

    std::println(std::clog, "Wall panel {} selected! Revealing compartment...",
                 obj->getName());

    state = State::Revealing;
    timeAccumulator = 0.0f;

    // Panel becomes non-interactable during animation
    obj->interactable = false;

    triggerArrowSystem();
  }

  void onUpdate(GameObject *obj, float deltaTime) override {
    if (state != State::Revealing) {
      return;
    }

    timeAccumulator += deltaTime;
    float t = glm::clamp(timeAccumulator / revealDuration, 0.0f, 1.0f);

    // Panel scales down (into wall)
    obj->scale = glm::vec3(0.2f, 0.2f, 0.001f) * (1.0f - t);

    // Cavity scales up
    if (cavityObj) {
      cavityObj->scale = cavityScale * t;
    }

    if (t >= 1.0f) {
      state = State::Revealed;
      std::println(std::clog, "Compartment fully revealed!");

      // Panel becomes invisible
      obj->scale = glm::vec3(0.001f);
    }
  }

  const char *getName() const override { return "WallPanelBehaviour"; }

private:
  TrapManager *trapManager;
  GameObject *cavityObj;
  std::function<void()> arrowCallback;
  glm::vec3 cavityScale;
  float revealDuration;
  State state;
  GameObject *obj;
  float timeAccumulator;

  void triggerArrowSystem() {
    std::println(std::clog, "Triggering arrow particle system");
    if (arrowCallback) {
      arrowCallback();
    } else {
      std::println(std::clog, "Warning: No arrow callback registered");
    }
  }
};
