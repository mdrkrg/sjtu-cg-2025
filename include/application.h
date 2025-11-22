#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include <mutex>

#include "camera.h"

class GraphicsRenderer;
class InputHandler;

class Application : public std::enable_shared_from_this<Application> {
public:
  /// Get the application instance
  inline static std::shared_ptr<Application> getInstance() {
    // called once, return the singleton in subsequent calls
    std::call_once(init_flag, []() {
      singleton = std::shared_ptr<Application>(new Application());
    });
    return singleton;
  }
  ~Application();
  Application(const Application &) = delete;
  Application &operator=(const Application &) = delete;

  /// Initialize the application
  bool initialize();

  /// Run the application
  void run();

  /// Clean up resources
  void cleanup();

  /// Take a screenshot
  void screenshot();

private:
  Application();
  /// Application singleton
  static std::shared_ptr<Application> singleton;
  static std::once_flag init_flag;

  /// Screen width
  static constexpr unsigned int SCR_WIDTH = 1920;
  /// Screen height
  static constexpr unsigned int SCR_HEIGHT = 1080;

  /// Window and context
  std::shared_ptr<GLFWwindow> window;

  // Camera
  Camera camera;

  /// Time delta between frames
  float deltaTime;

  /// Time of last frame
  float lastFrame;

  std::unique_ptr<InputHandler> inputHandler;

  // Graphics components
  std::unique_ptr<GraphicsRenderer> renderer;

  // Utilities
  void updateDeltaTime();
};

inline std::shared_ptr<Application> Application::singleton = nullptr;
inline std::once_flag Application::init_flag = std::once_flag{};
