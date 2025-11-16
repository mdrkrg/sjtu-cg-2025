#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "camera.h"
#include "cloud.h"

class InputHandler {
public:
  InputHandler(Camera &camera, float screenWidth, float screenHeight);

  void processInput(GLFWwindow *window, float deltaTime);
  void mouseCallback(double xpos, double ypos);
  void scrollCallback(double yoffset);
  void framebufferSizeCallback(int width, int height);

  void initCloud(std::shared_ptr<Cloud> &&cloud) {
    this->cloud = std::move(cloud);
  }

private:
  Camera &camera;

  std::shared_ptr<Cloud> cloud;
  bool cloudToggled;

  float screenWidth;
  float screenHeight;

  // Mouse input tracking
  float lastX;
  float lastY;
  bool firstMouse;

  void processCameraInput(GLFWwindow *window, float deltaTime);
  void processCloudInput(GLFWwindow *window, float deltaTime);
};
