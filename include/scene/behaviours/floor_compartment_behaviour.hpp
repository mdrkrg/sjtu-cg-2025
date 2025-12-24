#pragma once

#include "scene/game_object.hpp"
#include "scene/game_object_behaviour.hpp"
#include "scene/puzzle_manager.hpp"
#include <glm/glm.hpp>
#include <print>
#include <functional>

class FloorCompartmentBehaviour : public IGameObjectBehaviour {
public:
  enum class State {
    Locked,    // Cannot interact (puzzle not solved)
    Unlocked,  // Can interact, appears as floor
    Revealing, // Animation in progress
    Revealed   // Compartment visible, orb glowing
  };

  FloorCompartmentBehaviour(
      PuzzleManager *puzzleManager, GameObject *cavityObj, GameObject *orbObj,
      std::function<void()> glowCallback = nullptr,
      const glm::vec3 &cavityScale = glm::vec3(0.15f, 0.1f, 0.15f),
      float revealDuration = 1.0f)
      : puzzleManager{puzzleManager}, cavityObj{cavityObj}, orbObj{orbObj},
        glowCallback{glowCallback}, cavityScale{cavityScale},
        revealDuration{revealDuration}, state{State::Locked}, obj{nullptr},
        timeAccumulator{0.0f} {}

  ~FloorCompartmentBehaviour() = default;

  void onAttach(GameObject *obj) override {
    this->obj = obj;
    if (not puzzleManager) {
      std::println(
          std::cerr,
          "PuzzleManager not initialized in FloorCompartmentBehaviour when "
          "attaching {}",
          obj->getName());
      return;
    }

    std::println(std::clog, "FloorCompartmentBehaviour attached to {}",
                 obj->getName());

    // Register callback for when puzzle is solved
    puzzleManager->registerPuzzleCallback(this, [this] {
      std::println(
          std::clog,
          "FloorCompartmentBehaviour: Puzzle solved, compartment unlocked!");
      if (this->obj) {
        this->obj->interactable = true; // Now can be clicked
        state = State::Unlocked;
      }
    });
  }

  void onDetach(GameObject *) override {
    if (puzzleManager) {
      puzzleManager->unregisterPuzzleCallback(this);
    }
    this->obj = nullptr;
  }

  void onSelect(GameObject *obj) override {
    if (state != State::Unlocked) {
      std::println(std::clog, "Floor compartment {} not unlocked (state: {})",
                   obj->getName(), static_cast<int>(state));
      return;
    }

    std::println(std::clog, "Floor compartment {} selected! Revealing orb...",
                 obj->getName());

    state = State::Revealing;
    timeAccumulator = 0.0f;

    // Tile becomes non-interactable during animation
    obj->interactable = false;

    // Trigger orb glow effect
    triggerOrbGlow();
  }

  void onUpdate(GameObject *obj, float deltaTime) override {
    if (state != State::Revealing) {
      return;
    }

    timeAccumulator += deltaTime;
    float t = glm::clamp(timeAccumulator / revealDuration, 0.0f, 1.0f);

    // Tile rotates up (around X axis at one edge)
    // Position tile so bottom edge is at hinge, rotate -90 degrees
    float rotationAngle = -90.0f * t; // Rotate upward
    obj->rotation.x = rotationAngle;

    // Rotate around one axis
    // FIXME: Use an elegent solution
    glm::vec3 transform{0, 0.0001 * t, -0.0001 * t};
    obj->position += transform;

    // Cavity scales up (from floor level)
    if (cavityObj) {
      cavityObj->scale = cavityScale * t;
    }

    // Orb scales up and becomes visible
    if (orbObj) {
      // Orb grows
      float orbScale = 0.1f * t;
      orbObj->scale = glm::vec3(orbScale);

      // Make orb emissive
      if (orbObj->getMeshMaterialCount() > 0) {
        auto &mat = orbObj->getMeshMaterial(0);
        mat.setEmissive(true, glm::vec3(0.2f, 0.8f, 1.0f), 1.0f + t * 2.0f);
      }
    }

    if (t >= 1.0f) {
      state = State::Revealed;
      std::println(std::clog,
                   "Floor compartment fully revealed with glowing orb!");
    }
  }

  const char *getName() const override { return "FloorCompartmentBehaviour"; }

private:
  PuzzleManager *puzzleManager;
  GameObject *cavityObj;
  GameObject *orbObj;
  std::function<void()> glowCallback;
  glm::vec3 cavityScale;
  float revealDuration;
  State state;
  GameObject *obj;
  float timeAccumulator;

  void triggerOrbGlow() {
    std::println(std::clog, "Triggering orb glow effect");
    if (glowCallback) {
      glowCallback();
    } else {
      std::println(std::clog, "Warning: No glow callback registered");
    }
  }
};
