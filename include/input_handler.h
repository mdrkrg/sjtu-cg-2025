#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "camera.h"

class InputHandler {
public:
  InputHandler(Camera &camera, float screenWidth, float screenHeight);

  void processInput(GLFWwindow *window, float deltaTime);
  void mouseCallback(double xpos, double ypos);
  void scrollCallback(double yoffset);
  void framebufferSizeCallback(int width, int height);

private:
  Camera &camera;
  float screenWidth;
  float screenHeight;

  // Mouse input tracking
  float lastX;
  float lastY;
  bool firstMouse;
};
