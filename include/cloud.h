#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>

#include "shader.h"

const float CLOUD_SPEED = 1.0f;

class Cloud {

public:
  Cloud();
  ~Cloud();

  enum Movement { FORWARD, BACKWARD, LEFT, RIGHT };

  bool initialize();
  void render(const glm::mat4 &projection, const glm::mat4 &view,
              const glm::vec3 &lightPosition, const glm::vec3 &cameraPosition);
  void setPosition(const glm::vec3 &position);
  void setScale(const glm::vec3 &scale);
  void update(float deltaTime);
  void cleanup();

  bool isInitialized() const { return initialized; }

  void processKeyboard(Movement direction, float deltaTime);
  void toggle(bool toggled) { this->toggled = toggled; }

private:
  glm::vec3 position;
  glm::vec3 scale;

  float movementSpeed = CLOUD_SPEED;

  // Graphics
  unsigned int VAO, VBO;
  unsigned int volumeTexture;
  std::unique_ptr<Shader> cloudShader;

  // Layered billboard
  static const int SLICE_COUNT = 16;
  unsigned int sliceVAOs[SLICE_COUNT];
  unsigned int sliceVBOs[SLICE_COUNT];

  // State
  bool initialized;
  bool toggled = false;
  float animationTime;

  bool setupGeometry();
  bool generateVolumeTexture();
  void setupShader();
  void cleanupGeometry();

  // Noise generation for procedural cloud texture
  float noise(float x, float y, float z);
  float fractalNoise(float x, float y, float z, int octaves);
};
