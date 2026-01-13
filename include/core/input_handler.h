#pragma once

#include <epoxy/gl.h>
#include <epoxy/glx.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <unordered_set>
#include <functional>

#include "camera.h"
#include "graphics/cloud.h"

using graphics::particles::ParticleSystem;

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

  void initCloud(std::shared_ptr<Cloud> cloud) { this->cloud = cloud; }

  // Mouse interaction
  void setMouseClickCallback(std::function<void(const math::Ray &)> callback) {
    onMouseClick = callback;
  }
  void toggleCursor();

private:
  std::weak_ptr<GLFWwindow> window;
  Camera &camera;

  std::shared_ptr<Cloud> cloud;

  std::unordered_set<int> pressedKeys{};

  bool cloudToggled = false;

  float screenWidth;
  float screenHeight;

  // Mouse input tracking
  float lastX;
  float lastY;
  bool firstMouse;
  bool cursorCaptured = true;
  std::function<void(const math::Ray &)> onMouseClick;

  void processCameraInput(float deltaTime);
  void processCloudInput(float deltaTime);

  /// Get mouse ray at a certain screen positions from the camera position
  math::Ray getMouseRay(float x, float y) const;
};
