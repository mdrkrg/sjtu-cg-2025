#include "core/input_handler.h"
#include "core/application.h"

InputHandler::InputHandler(Camera &camera, float screenWidth,
                           float screenHeight)
    : camera(camera), screenWidth(screenWidth), screenHeight(screenHeight),
      lastX(screenWidth / 2.0f), lastY(screenHeight / 2.0f), firstMouse(true) {}

void InputHandler::install(std::shared_ptr<GLFWwindow> window) {
  this->window = window;
  glfwSetWindowUserPointer(window.get(), this);
  glfwSetKeyCallback(window.get(), [](GLFWwindow *w, int key, int scancode,
                                      int action, int mods) {
    static_cast<InputHandler *>(glfwGetWindowUserPointer(w))
        ->keyCallback(w, key, scancode, action, mods);
  });

  glfwSetCursorPosCallback(
      window.get(), [](GLFWwindow *w, double xposIn, double yposIn) {
        static_cast<InputHandler *>(glfwGetWindowUserPointer(w))
            ->mouseCallback(xposIn, yposIn);
      });

  glfwSetMouseButtonCallback(
      window.get(), [](GLFWwindow *w, int button, int action, int mods) {
        static_cast<InputHandler *>(glfwGetWindowUserPointer(w))
            ->mouseButtonCallback(w, button, action, mods);
      });

  // Capture mouse
  glfwSetInputMode(window.get(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  glfwSetFramebufferSizeCallback(window.get(),
                                 [](GLFWwindow *w, int width, int height) {
                                   glViewport(0, 0, width, height);
                                 });
  glfwSetScrollCallback(
      window.get(), [](GLFWwindow *w, double xoffset, double yoffset) {
        static_cast<InputHandler *>(glfwGetWindowUserPointer(w))
            ->scrollCallback(yoffset);
      });
}

void InputHandler::keyCallback(GLFWwindow *window, int key, int scancode,
                               int action, int mods) {
  if (action == GLFW_PRESS) {
    pressedKeys.insert(key);
  } else if (action == GLFW_RELEASE) {
    pressedKeys.erase(key);
  }
}

void InputHandler::update(float deltaTime) {
  if (pressed(GLFW_KEY_ESCAPE) and not window.expired()) {
    const auto w = window.lock();
    glfwSetWindowShouldClose(w.get(), true);
  }

  if (pressed(GLFW_KEY_F12)) {
    Application::getInstance()->screenshot();
    pressedKeys.erase(GLFW_KEY_F12);
  }

  if (pressed(GLFW_KEY_F3)) {
    Application::getInstance()->debug();
    pressedKeys.erase(GLFW_KEY_F3);
  }

  if (pressed(GLFW_KEY_TAB)) {
    toggleCursor();
    pressedKeys.erase(GLFW_KEY_TAB);
  }

  if (not cloud) {
    processCameraInput(deltaTime);
    return;
  }

  // TODO: make control more human
  if (cloud and pressed(GLFW_KEY_M)) {
    cloudToggled = not cloudToggled;
    snowToggled = false;
    rainToggled = false;
    cloud->toggle(cloudToggled);
    rainSystem->toggle(rainToggled);
    snowSystem->toggle(snowToggled);
    pressedKeys.erase(GLFW_KEY_M);
  }

  if (not cloudToggled) {
    processCameraInput(deltaTime);
    return;
  }

  if (cloud and cloudToggled and pressed(GLFW_KEY_S)) {
    snowToggled = not snowToggled;
    snowSystem->toggle(snowToggled);
    pressedKeys.erase(GLFW_KEY_S);
  }

  if (cloud and cloudToggled and pressed(GLFW_KEY_R)) {
    rainToggled = not rainToggled;
    rainSystem->toggle(rainToggled);
    pressedKeys.erase(GLFW_KEY_R);
  }

  processCloudInput(deltaTime);
  // TODO: Make this consistant with the cloud.
  // Logically, the control should be on the cloud side.
  if (snowToggled) {
    processSnowInput(deltaTime);
  }
  if (rainToggled) {
    processRainInput(deltaTime);
  }
}

void InputHandler::processCameraInput(float deltaTime) {
  if (pressed(GLFW_KEY_W)) {
    camera.ProcessKeyboard(FORWARD, deltaTime);
  }
  if (pressed(GLFW_KEY_S)) {
    camera.ProcessKeyboard(BACKWARD, deltaTime);
  }
  if (pressed(GLFW_KEY_A)) {
    camera.ProcessKeyboard(LEFT, deltaTime);
  }
  if (pressed(GLFW_KEY_D)) {
    camera.ProcessKeyboard(RIGHT, deltaTime);
  }
}

void InputHandler::processCloudInput(float deltaTime) {
  if (not cloud) {
    return;
  }
  if (pressed(GLFW_KEY_W)) {
    cloud->processKeyboard(FORWARD, deltaTime);
  }
  if (pressed(GLFW_KEY_X)) {
    cloud->processKeyboard(BACKWARD, deltaTime);
  }
  if (pressed(GLFW_KEY_A)) {
    cloud->processKeyboard(LEFT, deltaTime);
  }
  if (pressed(GLFW_KEY_D)) {
    cloud->processKeyboard(RIGHT, deltaTime);
  }
}

void InputHandler::processRainInput(float deltaTime) {
  if (not rainSystem) {
    return;
  }
  if (pressed(GLFW_KEY_W)) {
    rainSystem->processKeyboard(FORWARD, deltaTime);
  }
  if (pressed(GLFW_KEY_X)) {
    rainSystem->processKeyboard(BACKWARD, deltaTime);
  }
  if (pressed(GLFW_KEY_A)) {
    rainSystem->processKeyboard(LEFT, deltaTime);
  }
  if (pressed(GLFW_KEY_D)) {
    rainSystem->processKeyboard(RIGHT, deltaTime);
  }
}

void InputHandler::processSnowInput(float deltaTime) {
  if (not snowSystem) {
    return;
  }
  if (pressed(GLFW_KEY_W)) {
    snowSystem->processKeyboard(FORWARD, deltaTime);
  }
  if (pressed(GLFW_KEY_X)) {
    snowSystem->processKeyboard(BACKWARD, deltaTime);
  }
  if (pressed(GLFW_KEY_A)) {
    snowSystem->processKeyboard(LEFT, deltaTime);
  }
  if (pressed(GLFW_KEY_D)) {
    snowSystem->processKeyboard(RIGHT, deltaTime);
  }
}

void InputHandler::getMouseRay(float x, float y, glm::vec3 &rayOrigin,
                               glm::vec3 &rayDir) const {
  // Convert screen coordinates to normalized device coordinates
  float ndcX = (2.0f * x) / screenWidth - 1.0f;
  float ndcY = 1.0f - (2.0f * y) / screenHeight; // Y inverted

  // Convert NDC to view space ray
  float aspect = screenWidth / screenHeight;
  float fovRad = glm::radians(camera.Zoom);
  float tanHalfFov = tan(fovRad * 0.5f);

  glm::vec3 rayView = glm::normalize(
      glm::vec3(ndcX * aspect * tanHalfFov, ndcY * tanHalfFov, -1.0f));

  // Transform ray to world space using camera orientation
  glm::mat4 viewMatrix = camera.GetViewMatrix();
  glm::mat4 invView = glm::inverse(viewMatrix);
  glm::vec4 rayWorld = invView * glm::vec4(rayView, 0.0f);
  rayDir = glm::normalize(glm::vec3(rayWorld));

  // Ray origin is camera position
  rayOrigin = camera.Position;
}

void InputHandler::mouseCallback(double xpos, double ypos) {
  if (!cursorCaptured)
    return;
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

void InputHandler::mouseButtonCallback(GLFWwindow *window, int button,
                                       int action, int mods) {
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    if (!cursorCaptured) {
      double xpos, ypos;
      glfwGetCursorPos(window, &xpos, &ypos);
      if (onMouseClick) {
        glm::vec3 rayOrigin, rayDir;
        getMouseRay(static_cast<float>(xpos), static_cast<float>(ypos),
                    rayOrigin, rayDir);
        onMouseClick(rayOrigin, rayDir);
      }
    }
  }
}

void InputHandler::toggleCursor() {
  if (window.expired())
    return;
  auto w = window.lock();
  cursorCaptured = !cursorCaptured;
  glfwSetInputMode(w.get(), GLFW_CURSOR,
                   cursorCaptured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
  // Reset firstMouse to avoid jump when re-capturing
  if (cursorCaptured) {
    firstMouse = true;
  }
}

void InputHandler::scrollCallback(double yoffset) {
  camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void InputHandler::framebufferSizeCallback(int width, int height) {
  glViewport(0, 0, width, height);
}
