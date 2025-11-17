#include "input_handler.h"

InputHandler::InputHandler(Camera &camera, float screenWidth,
                           float screenHeight)
    : camera(camera), screenWidth(screenWidth), screenHeight(screenHeight),
      lastX(screenWidth / 2.0f), lastY(screenHeight / 2.0f), firstMouse(true) {}

void InputHandler::processInput(GLFWwindow *window, float deltaTime) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }

  // TODO: eliminate key flickering and unify key-bind
  if (cloud and not cloudToggled and
      glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
    cloudToggled = true;
    cloud->toggle(cloudToggled);
  }

  if (cloud and cloudToggled and glfwGetKey(window, GLFW_KEY_END)) {
    cloudToggled = false;
    snowToggled = false;
    rainToggled = false;
    cloud->toggle(cloudToggled);
    rainSystem->toggle(rainToggled);
    snowSystem->toggle(snowToggled);
  }

  if (cloud and cloudToggled and glfwGetKey(window, GLFW_KEY_S)) {
    snowToggled = not snowToggled;
    snowSystem->toggle(snowToggled);
  }

  if (cloud and cloudToggled and glfwGetKey(window, GLFW_KEY_R)) {
    rainToggled = not rainToggled;
    rainSystem->toggle(rainToggled);
  }

  if (not cloud) {
    processCameraInput(window, deltaTime);
    return;
  }

  if (cloudToggled) {
    processCloudInput(window, deltaTime);
    // TODO: Make this consistant with the cloud.
    // Logically, the control should be on the cloud side.
    if (snowToggled) {
      processSnowInput(window, deltaTime);
    }
    if (rainToggled) {
      processRainInput(window, deltaTime);
    }
  } else {
    processCameraInput(window, deltaTime);
  }
}

void InputHandler::processCameraInput(GLFWwindow *window, float deltaTime) {
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    camera.ProcessKeyboard(FORWARD, deltaTime);
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    camera.ProcessKeyboard(BACKWARD, deltaTime);
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    camera.ProcessKeyboard(LEFT, deltaTime);
  }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    camera.ProcessKeyboard(RIGHT, deltaTime);
  }
}

void InputHandler::processCloudInput(GLFWwindow *window, float deltaTime) {
  if (not cloud) {
    return;
  }
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    cloud->processKeyboard(FORWARD, deltaTime);
  }
  if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
    cloud->processKeyboard(BACKWARD, deltaTime);
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    cloud->processKeyboard(LEFT, deltaTime);
  }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    cloud->processKeyboard(RIGHT, deltaTime);
  }
}

void InputHandler::processRainInput(GLFWwindow *window, float deltaTime) {
  if (not rainSystem) {
    return;
  }
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    rainSystem->processKeyboard(FORWARD, deltaTime);
  }
  if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
    rainSystem->processKeyboard(BACKWARD, deltaTime);
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    rainSystem->processKeyboard(LEFT, deltaTime);
  }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    rainSystem->processKeyboard(RIGHT, deltaTime);
  }
}

void InputHandler::processSnowInput(GLFWwindow *window, float deltaTime) {
  if (not snowSystem) {
    return;
  }
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    snowSystem->processKeyboard(FORWARD, deltaTime);
  }
  if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
    snowSystem->processKeyboard(BACKWARD, deltaTime);
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    snowSystem->processKeyboard(LEFT, deltaTime);
  }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    snowSystem->processKeyboard(RIGHT, deltaTime);
  }
}

void InputHandler::mouseCallback(double xpos, double ypos) {
  float xposIn = static_cast<float>(xpos);
  float yposIn = static_cast<float>(ypos);

  if (firstMouse) {
    lastX = xposIn;
    lastY = yposIn;
    firstMouse = false;
  }

  float xoffset = xposIn - lastX;
  // Reversed since y-coordinates go from bottom to top
  float yoffset = lastY - yposIn;

  lastX = xposIn;
  lastY = yposIn;

  camera.ProcessMouseMovement(xoffset, yoffset);
}

void InputHandler::scrollCallback(double yoffset) {
  camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void InputHandler::framebufferSizeCallback(int width, int height) {
  glViewport(0, 0, width, height);
}
