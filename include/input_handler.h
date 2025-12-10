#pragma once

#include <epoxy/gl.h>
#include <epoxy/glx.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <unordered_set>

#include "camera.h"
#include "cloud.h"
#include "particles/particle_system.hpp"

class InputHandler {
public:
  InputHandler(Camera &camera, float screenWidth, float screenHeight);

  void install(std::shared_ptr<GLFWwindow> window);

  bool pressed(int key) { return pressedKeys.contains(key); }

  void keyCallback(GLFWwindow *window, int key, int scancode, int action,
                   int mods);
  void update(float deltaTime);
  void mouseCallback(double xpos, double ypos);
  void scrollCallback(double yoffset);
  void framebufferSizeCallback(int width, int height);

  void initWeather(std::shared_ptr<Cloud> &&cloud,
                   std::shared_ptr<ParticleSystem> &&rainSystem,
                   std::shared_ptr<ParticleSystem> &&snowSystem) {
    this->cloud = std::move(cloud);
    this->rainSystem = std::move(rainSystem);
    this->snowSystem = std::move(snowSystem);
  }

private:
  std::weak_ptr<GLFWwindow> window;
  Camera &camera;

  std::shared_ptr<Cloud> cloud;
  std::shared_ptr<ParticleSystem> rainSystem;
  std::shared_ptr<ParticleSystem> snowSystem;

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

  void processCameraInput(float deltaTime);
  void processCloudInput(float deltaTime);
  void processRainInput(float deltaTime);
  void processSnowInput(float deltaTime);
};
