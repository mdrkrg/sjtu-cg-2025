#pragma once

#include <epoxy/gl.h>
#include <epoxy/glx.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <unordered_set>
#include <functional>

#include "camera.h"
#include "graphics/cloud.h"
#include "graphics/particles/particle_system.hpp"

class InputHandler {
public:
  InputHandler(Camera &camera, float screenWidth, float screenHeight);

  void install(std::shared_ptr<GLFWwindow> window);

  bool pressed(int key) { return pressedKeys.contains(key); }

  void keyCallback(GLFWwindow *window, int key, int scancode, int action,
                   int mods);
  void update(float deltaTime);
  void mouseCallback(double xpos, double ypos);
  void mouseButtonCallback(GLFWwindow *window, int button, int action,
                           int mods);
  void scrollCallback(double yoffset);
  void framebufferSizeCallback(int width, int height);

  void initWeather(std::shared_ptr<Cloud> &&cloud,
                   std::shared_ptr<ParticleSystem<Particle>> &&rainSystem,
                   std::shared_ptr<ParticleSystem<Particle>> &&snowSystem) {
    this->cloud = std::move(cloud);
    this->rainSystem = std::move(rainSystem);
    this->snowSystem = std::move(snowSystem);
  }

  // Mouse interaction
  void setMouseClickCallback(
      std::function<void(const glm::vec3 &, const glm::vec3 &)> callback) {
    onMouseClick = callback;
  }
  void toggleCursor();

private:
  std::weak_ptr<GLFWwindow> window;
  Camera &camera;

  std::shared_ptr<Cloud> cloud;
  std::shared_ptr<ParticleSystem<Particle>> rainSystem;
  std::shared_ptr<ParticleSystem<Particle>> snowSystem;

  std::unordered_set<int> pressedKeys{};

  bool cloudToggled = false;
  bool rainToggled = false;
  bool snowToggled = false;

  float screenWidth;
  float screenHeight;

  // Mouse input tracking
  float lastX;
  float lastY;
  bool firstMouse;
  bool cursorCaptured = true;
  std::function<void(const glm::vec3 &, const glm::vec3 &)> onMouseClick;

  void processCameraInput(float deltaTime);
  void processCloudInput(float deltaTime);

  /// Get mouse ray at a certain screen positions from the camera position
  void getMouseRay(float x, float y, glm::vec3 &rayOrigin,
                   glm::vec3 &rayDir) const;
};
