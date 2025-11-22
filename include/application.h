#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include <mutex>

#include "camera.h"

// Forward declarations
class GraphicsRenderer;
class InputHandler;

class Application : public std::enable_shared_from_this<Application> {
public:
  static std::shared_ptr<Application> getInstance() {
    // called once, return the singleton in subsequent calls
    std::call_once(init_flag, []() {
      singleton = std::shared_ptr<Application>(new Application());
    });
    return singleton;
  }
  ~Application();
  Application(const Application &) = delete;
  Application &operator=(const Application &) = delete;

  bool initialize();
  void run();
  void cleanup();

  void screenshot();

private:
  Application();
  static std::shared_ptr<Application> singleton;
  static std::once_flag init_flag;

  // Window settings
  static constexpr unsigned int SCR_WIDTH = 1920;
  static constexpr unsigned int SCR_HEIGHT = 1080;

  // Window and context
  std::shared_ptr<GLFWwindow> window;

  // Camera
  Camera camera;

  // Timing
  float deltaTime;
  float lastFrame;

  std::unique_ptr<InputHandler> inputHandler;

  // Light and object positions
  glm::vec3 lightPos;
  glm::vec3 cubePos;
  glm::vec3 tablePos;

  // Graphics components
  std::unique_ptr<GraphicsRenderer> renderer;

  // Utilities
  void updateDeltaTime();
};

inline std::shared_ptr<Application> Application::singleton = nullptr;
inline std::once_flag Application::init_flag = std::once_flag{};
