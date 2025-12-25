#pragma once

#include <glm/glm.hpp>
#include <epoxy/gl.h>
#include <epoxy/glx.h>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>

#include "core/movement.hpp"
#include "scene/game_object.hpp"
#include "shader.h"

const float CLOUD_SPEED = 1.0f;

class Cloud {

public:
  Cloud();
  ~Cloud();

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

  const GameObject *getGameObject() const { return obj.get(); }

private:
  float movementSpeed = CLOUD_SPEED;

  // Graphics

  unsigned int VAO, VBO;
  unsigned int volumeTexture;
  std::shared_ptr<Shader> cloudShader;

  /// Layered billboard slice count
  static const int SLICE_COUNT = 16;
  unsigned int sliceVAOs[SLICE_COUNT];
  unsigned int sliceVBOs[SLICE_COUNT];

  // State

  bool initialized;
  bool toggled = false;
  float animationTime;

  std::unique_ptr<GameObject> obj;

  bool setupGeometry();
  bool generateVolumeTexture();
  void setupShader();
  void cleanupGeometry();
};
