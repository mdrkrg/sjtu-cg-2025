#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <memory>

#include "camera.h"
#include "cloud.h"
#include "particles/particle_system.hpp"

class InputHandler {
public:
  InputHandler(Camera &camera, float screenWidth, float screenHeight);

  void processInput(GLFWwindow *window, float deltaTime);
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
  Camera &camera;

  std::shared_ptr<Cloud> cloud;
  std::shared_ptr<ParticleSystem> rainSystem;
  std::shared_ptr<ParticleSystem> snowSystem;

  bool cloudToggled = false;
  bool rainToggled = false;
  bool snowToggled = false;

  float screenWidth;
  float screenHeight;

  // Mouse input tracking
  float lastX;
  float lastY;
  bool firstMouse;

  void processCameraInput(GLFWwindow *window, float deltaTime);
  void processCloudInput(GLFWwindow *window, float deltaTime);
  void processRainInput(GLFWwindow *window, float deltaTime);
  void processSnowInput(GLFWwindow *window, float deltaTime);
};
