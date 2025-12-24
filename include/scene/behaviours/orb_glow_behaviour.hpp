#pragma once

#include "scene/game_object.hpp"
#include "scene/game_object_behaviour.hpp"
#include <glm/glm.hpp>
#include <iostream>
#include <mutex>

/// Spirit orb glow behaviour
/// Pure glow effects: pulsating intensity, rotation, bobbing motion
class OrbGlowBehaviour : public IGameObjectBehaviour {
public:
  OrbGlowBehaviour(const glm::vec3 &glowColor = glm::vec3(0.2f, 0.8f, 1.0f),
                   float pulseSpeed = 2.0f, float pulseAmount = 0.3f)
      : glowColor(glowColor), baseIntensity(1.0f), pulseSpeed(pulseSpeed),
        pulseAmount(pulseAmount), isActivated(false), timeAccumulator(0.0f) {
    std::cout << "OrbGlowBehaviour created with glow color (" << glowColor.r
              << ", " << glowColor.g << ", " << glowColor.b << ")" << std::endl;
  }

  void onSelect(GameObject *obj) override {
    if (isActivated) {
      std::cout << "Spirit orb " << obj->getName() << " already activated"
                << std::endl;
      return;
    }

    isActivated = true;
    std::cout << "Spirit orb " << obj->getName()
              << " selected! Glow intensified." << std::endl;

    // Increase glow intensity when activated
    baseIntensity = 2.0f;
    pulseAmount = 0.5f;
  }

  void onHover(GameObject *obj, bool enter) override {
    if (enter) {
      std::cout << "Hovering over spirit orb " << obj->getName() << " ("
                << (isActivated ? "activated" : "inactive") << ")" << std::endl;
    }
  }

  // Store original Y position on first call
  void init(GameObject *obj) {
    std::call_once(firstCall, [this, &obj] { originalY = obj->position.y; });
  }

  void onUpdate(GameObject *obj, float deltaTime) override {
    init(obj);

    timeAccumulator += deltaTime;

    // Calculate pulsating intensity
    float pulse = sin(timeAccumulator * pulseSpeed) * pulseAmount;
    currentIntensity = baseIntensity + pulse;

    // Slowly rotate orb for visual effect
    obj->rotation.y += deltaTime * 30.0f; // 30 degrees per second

    // Bobbing motion when not activated
    if (not isActivated) {
      float bob = sin(timeAccumulator * 1.5f) * 0.02f + 0.02f;
      obj->position.y = originalY + bob;
    }
  }

  void onPreRender(GameObject *obj) override {
    init(obj);

    // Apply emission to the orb's material
    size_t materialCount = obj->getMeshMaterialCount();
    if (materialCount > 0) {
      // Update first material
      auto &material = obj->getMeshMaterial(0);
      material.setEmissive(true, glowColor, currentIntensity);
    }
  }

  const char *getName() const override { return "OrbGlowBehaviour"; }

private:
  glm::vec3 glowColor; // Cyan
  float baseIntensity;
  float pulseSpeed;
  float pulseAmount;
  float currentIntensity = 1.0f;
  bool isActivated = false;
  float timeAccumulator = 0.0f;
  float originalY = 0.0f;
  std::once_flag firstCall = std::once_flag{};
};
