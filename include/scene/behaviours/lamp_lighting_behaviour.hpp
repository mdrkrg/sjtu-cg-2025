#pragma once

#include "scene/game_object.hpp"
#include "scene/game_object_behaviour.hpp"
#include <glm/glm.hpp>
#include <iostream>

/// Lamp lighting behaviour (Strategy Pattern example)
/// Toggles light on/off when selected, manages light source
class LampLightingBehaviour : public IGameObjectBehaviour {
public:
  LampLightingBehaviour(const glm::vec3 &lightColor = glm::vec3(1.0f, 0.9f,
                                                                0.6f),
                        float intensity = 1.0f)
      : lightColor(lightColor), intensity(intensity), lightOn(false) {
    std::cout << "LampLightingBehaviour created with color (" << lightColor.r
              << ", " << lightColor.g << ", " << lightColor.b << ")"
              << std::endl;
  }

  void onSelect(GameObject *obj) override {
    lightOn = !lightOn;
    std::cout << "Lamp " << obj->getName() << " toggled "
              << (lightOn ? "ON" : "OFF") << std::endl;

    // In a full implementation, would:
    // 1. Update GameObject material.emissive = lightOn
    // 2. Update GameObject material.emissionColor = lightColor
    // 3. Add/remove light source from scene via GameManager
    // 4. Update shader uniforms for emission

    // For demonstration, just print action
    if (lightOn) {
      std::cout << "  - Light position: " << obj->position.x << ", "
                << obj->position.y << ", " << obj->position.z << std::endl;
      std::cout << "  - Light color: (" << lightColor.r << ", " << lightColor.g
                << ", " << lightColor.b << ")" << std::endl;
    }
  }

  void onHover(GameObject *obj, bool enter) override {
    if (enter) {
      std::cout << "Hovering over lamp " << obj->getName() << " (light is "
                << (lightOn ? "ON" : "OFF") << ")" << std::endl;
    }
  }

  void onUpdate(GameObject *obj, float deltaTime) override {
    // Could implement light flicker animation here
    // For example: intensity = baseIntensity + sin(time * flickerSpeed) *
    // flickerAmount
    timeAccumulator += deltaTime;

    // Simple demonstration: pulse intensity when light is on
    if (lightOn) {
      float pulse = 0.5f + 0.5f * sin(timeAccumulator * 2.0f);
      currentIntensity = intensity * pulse;
    } else {
      currentIntensity = 0.0f;
    }
  }

  void onPreRender(GameObject *obj) override {
    // In full implementation, would set shader uniforms:
    // shader.setBool("material.emissive", lightOn);
    // shader.setVec3("material.emission", lightColor * currentIntensity);

    // For demonstration, just track that we're about to render
    // (could set a flag on GameObject if it had material properties)
  }

  void onPostRender(GameObject *obj) override {
    // Could reset any temporary shader modifications here
  }

  const char *getName() const override { return "LampLightingBehaviour"; }

  // Getters for light state
  bool isLightOn() const { return lightOn; }
  glm::vec3 getLightColor() const { return lightColor; }
  float getCurrentIntensity() const { return currentIntensity; }

  // Setters
  void setLightColor(const glm::vec3 &color) { lightColor = color; }
  void setIntensity(float newIntensity) { intensity = newIntensity; }

private:
  glm::vec3 lightColor; // Warm yellow default
  float intensity;      // Base intensity
  float currentIntensity = 0.0f;
  bool lightOn = false;
  float timeAccumulator = 0.0f;
};
