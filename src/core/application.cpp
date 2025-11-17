#include "application.h"
#include "input_handler.h"
#include "graphics_renderer.h"
#include <iostream>
#include <memory>
#include <stb_image.h>

// Static member definitions
Application::Application()
    : window(nullptr), camera(glm::vec3(0.0f, 0.3f, 3.3f)), deltaTime(0.0f),
      lastFrame(0.0f), inputHandler(std::make_unique<InputHandler>(
                           camera, SCR_WIDTH, SCR_HEIGHT)),
      lightPos(0.0f, 0.75f, 1.65f), cubePos(0.0f, 0.3f, 2.0f),
      tablePos(0.0f, -0.2f, 2.0f) {}

Application::~Application() {
  cleanup();
  Application::singleton = nullptr;
}

bool Application::initialize() {
  // Initialize GLFW
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // Set texture loading flag
  stbi_set_flip_vertically_on_load(true);

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  // Create window
  window.reset(
      glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", nullptr, nullptr),
      [](GLFWwindow *window) { glfwDestroyWindow(window); });
  if (window == nullptr) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return false;
  }

  glfwMakeContextCurrent(window.get());
  inputHandler->install(window);

  // Initialize GLAD
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return false;
  }

  // Enable depth testing
  glEnable(GL_DEPTH_TEST);

  // Enable blending for transparency
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Initialize renderer
  renderer = std::make_unique<GraphicsRenderer>();
  if (!renderer->initialize()) {
    std::cout << "Failed to initialize renderer" << std::endl;
    return false;
  }

  inputHandler->initWeather(renderer->getCloud(), renderer->getRainSystem(),
                            renderer->getSnowSystem());

  return true;
}

void Application::run() {
  // Render loop
  while (!glfwWindowShouldClose(window.get())) {
    updateDeltaTime();

    // Handle input
    inputHandler->update(deltaTime);

    // Update scene
    renderer->update(deltaTime);

    // Clear screen
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render scene
    glm::mat4 projection =
        glm::perspective(glm::radians(camera.Zoom),
                         (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

    glm::mat4 view = camera.GetViewMatrix();

    renderer->render(projection, view, camera.Position, lightPos);

    // Swap buffers and poll events
    glfwSwapBuffers(window.get());
    glfwPollEvents();
  }
}

void Application::cleanup() {
  renderer.reset();
  window.reset();
  glfwTerminate();
}

void Application::updateDeltaTime() {
  float currentFrame = static_cast<float>(glfwGetTime());
  deltaTime = currentFrame - lastFrame;
  lastFrame = currentFrame;
}
