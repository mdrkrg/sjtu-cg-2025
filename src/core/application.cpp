#include "core/application.h"
#include "core/input_handler.h"
#include "graphics/graphics_renderer.h"
#include "scene/game_manager.hpp"
#include <filesystem>
#include <iostream>
#include <memory>
#include <stb_image.h>

// Static member definitions
Application::Application()
    : window(nullptr), camera(glm::vec3(0.0f, 0.3f, 3.3f)), deltaTime(0.0f),
      lastFrame(0.0f), inputHandler(std::make_unique<InputHandler>(
                           camera, SCR_WIDTH, SCR_HEIGHT)),
      gameManager(std::make_shared<GameManager>()) {}

Application::~Application() {
  cleanup();
  Application::singleton = nullptr;
}

bool Application::initialize() {
  { // Init GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
  }

  // For texture loading flag
  stbi_set_flip_vertically_on_load(false);

  // Create window
  window.reset(
      glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", nullptr, nullptr),
      [](GLFWwindow *window) { glfwDestroyWindow(window); });
  if (window == nullptr) {
    std::println(std::cerr, "Failed to create GLFW window");
    glfwTerminate();
    return false;
  }

  glfwMakeContextCurrent(window.get());
  inputHandler->install(window);

  // Enable depth testing
  glEnable(GL_DEPTH_TEST);

  // Enable blending for transparency
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Initialize renderer
  renderer = std::make_unique<GraphicsRenderer>(gameManager);
  if (not renderer->initialize()) {
    std::println(std::cerr, "Failed to initialize renderer");
    return false;
  }

  // Pass weather into input handler
  inputHandler->initCloud(renderer->getCloud());

  // Set mouse click callback for object interaction
  inputHandler->setMouseClickCallback(
      [this](const math::Ray &ray) { renderer->handleMouseClick(ray); });

  return true;
}

void Application::run() {
  // Render loop
  while (not glfwWindowShouldClose(window.get())) {
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

    renderer->render(projection, view, camera.Position);

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

void Application::screenshot() {
  const int num = SCR_WIDTH * SCR_HEIGHT * 3;
  std::vector<uint8_t> pixels(num);

  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glReadBuffer(GL_BACK);
  glReadPixels(0, 0, SCR_WIDTH, SCR_HEIGHT, GL_BGR, GL_UNSIGNED_BYTE,
               pixels.data());

  std::time_t time = std::time({});
  char timeString[std::size("yyyy-mm-ddThh:mm:ssZ")];
  std::strftime(std::data(timeString), std::size(timeString), "%FT%TZ",
                std::gmtime(&time));
  {
    using namespace std::filesystem;
    const auto dir = "screenshots";
    if (not exists(dir)) {
      create_directories(dir);
    } else if (not is_directory(dir)) {
      std::println(std::cerr,
                   "Refuse to save screenshots to a non-directory file {}",
                   dir);
      return;
    }
  }

  FILE *outputFile = fopen(
      ("screenshots/screenshot-" + std::string{timeString} + ".tga").c_str(),
      "w");
  int16_t header[] = {0,
                      2,
                      0,
                      0,
                      0,
                      0,
                      static_cast<int16_t>(SCR_WIDTH),
                      static_cast<int16_t>(SCR_HEIGHT),
                      24};

  fwrite(&header, sizeof(header), 1, outputFile);
  fwrite(pixels.data(), num, 1, outputFile);
  fclose(outputFile);
}
