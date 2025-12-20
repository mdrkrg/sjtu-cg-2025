#pragma once

#include "scene/game_object.hpp"
#include "scene/game_object_behaviour.hpp"
#include "graphics/light_manager.hpp"
#include <glm/glm.hpp>
#include <iostream>

/// Lamp lighting behaviour (Strategy Pattern example)
/// Toggles light on/off when selected, manages light source
class LampLightingBehaviour : public IGameObjectBehaviour {
public:
  LampLightingBehaviour(graphics::LightManager &lightManager,
                        const glm::vec3 &lightColor = glm::vec3(1.0f, 0.9f,
                                                                0.6f),
                        float intensity = 1.0f, float lightRadius = 2.0f)
      : lightManager{lightManager}, lightColor{lightColor},
        intensity{intensity}, lightRadius{lightRadius}, lightOn{false},
        lightIndex{std::nullopt} {
    std::println(std::clog,
                 "LampLightingBehaviour created with color ({}, {}, {})",
                 lightColor.r, lightColor.g, lightColor.b);
  }

  ~LampLightingBehaviour() override {
    // Clean up light
    if (lightOn && lightIndex.has_value()) {
      lightManager.removePointLight(lightIndex.value());
      std::println(std::clog,
                   "LampLightingBehaviour destroyed, removed light at index {}",
                   lightIndex.value());
    }
  }

  void onSelect(GameObject *obj) override {
    lightOn = !lightOn;
    std::println(std::clog, "Lamp {} toggled {}", obj->getName(),
                 lightOn ? "ON" : "OFF");

    if (lightOn) {
      // Add point light to LightManager
      glm::vec3 lightPos = obj->position + glm::vec3(0.0f, 0.2f, 0.0f);
      graphics::PointLight lampLight(
          lightPos,                          // position
          lightColor * 0.05f,                // ambient (5% of color)
          lightColor * intensity,            // diffuse
          lightColor * 0.4f,                 // specular (reduced from 0.8)
          1.0f,                              // constant
          0.7f / lightRadius,                // linear
          1.8f / (lightRadius * lightRadius) // quadratic
      );

      lightIndex = lightManager.addPointLight(lampLight);
      if (not lightIndex.has_value()) {
        std::println(std::cerr,
                     "ERROR: Failed to add lamp light (max lights reached)");
        lightOn = false;
        return;
      }

      std::println(std::clog, "  - Light added at index {}",
                   lightIndex.value());
      std::println(std::clog, "  - Position: ({}, {}, {})", lightPos.x,
                   lightPos.y, lightPos.z);
      std::println(std::clog, "  - Color: ({}, {}, {})", lightColor.r,
                   lightColor.g, lightColor.b);
    } else {
      // Remove point light from LightManager
      if (lightIndex.has_value()) {
        lightManager.removePointLight(lightIndex.value());
        std::println(std::clog, "Light removed from index {}",
                     lightIndex.value());
        lightIndex = std::nullopt;
      }
    }

    // Update material emission
    updateMaterialEmission(obj);
  }

  void onHover(GameObject *obj, bool enter) override {
    if (enter) {
      std::println(std::clog, "Hovering over lamp {} (light is {})",
                   obj->getName(), lightOn ? "ON" : "OFF");
    }
  }

  void onUpdate(GameObject *obj, float deltaTime) override {
    // Update time accumulator for animation
    timeAccumulator += deltaTime;

    // Calculate current intensity with optional flicker
    if (lightOn) {
      // Simple flicker: 10% intensity variation
      float flicker = 0.9f + 0.2f * sin(timeAccumulator * 5.0f) *
                                 sin(timeAccumulator * 7.0f);
      currentIntensity = intensity * flicker;

      // Update light position if lamp moved
      if (lightIndex.has_value()) {
        updateLightPosition(obj);
      }
    } else {
      currentIntensity = 0.0f;
    }
  }

  void onPreRender(GameObject *obj) override {
    // Update material emission before rendering
    updateMaterialEmission(obj);
  }

  void onPostRender(GameObject *obj) override {
    // Nothing needed for now
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
  void updateMaterialEmission(GameObject *obj) {
    if (!obj)
      return;

    Material &mat = obj->getMaterial();
    if (lightOn) {
      mat.setEmissive(true, lightColor, currentIntensity);
    } else {
      mat.setEmissive(false);
    }
  }

  void updateLightPosition(GameObject *obj) {
    if (not lightIndex.has_value())
      return;

    glm::vec3 lightPos = obj->position + glm::vec3(0.0f, 0.2f, 0.0f);
    graphics::PointLight &light =
        lightManager.getPointLight(lightIndex.value());
    // Mark LightManager as dirty for update
    light.position = glm::vec4(lightPos, 1.0f);
    lightManager.updateUBO(); // Force update
  }

  // Member variables
  graphics::LightManager &lightManager;
  glm::vec3 lightColor; // Warm yellow default
  float intensity;      // Base intensity
  float lightRadius;    // Light influence radius
  float currentIntensity = 0.0f;
  bool lightOn = false;
  // Index in LightManager array, nullopt if not active
  std::optional<uint32_t> lightIndex;
  float timeAccumulator = 0.0f;
};
