#include <iostream>
#include <cmath>
#include <print>
#include <vector>
#include <algorithm>
#include "cloud.h"
#include "PerlinNoise.hpp"

float lerp(float t, float a, float b);

Cloud::Cloud()
    : position(0.0f, 0.0f, 0.0f), scale(1.0f, 0.5f, 1.0f), VAO(0), VBO(0),
      volumeTexture(0), initialized(false), animationTime(0.0f) {
  for (int i = 0; i < SLICE_COUNT; i++) {
    sliceVAOs[i] = 0;
    sliceVBOs[i] = 0;
  }
}

Cloud::~Cloud() { cleanup(); }

bool Cloud::initialize() {
  if (initialized) {
    return true;
  }

  // Setup shader
  try {
    setupShader();
  } catch (const std::exception &e) {
    std::println(std::cerr, "Failed to setup volumetric cloud shader: {}",
                 e.what());
    return false;
  }

  // Setup geometry
  if (not setupGeometry()) {
    std::println(std::cerr, "Failed to setup volumetric cloud geometry");
    return false;
  }

  // Generate volume texture
  if (not generateVolumeTexture()) {
    std::println(std::cerr, "Failed to generate volume texture");
    return false;
  }

  initialized = true;
  return true;
}

void Cloud::render(const glm::mat4 &projection, const glm::mat4 &view,
                   const glm::vec3 &lightPosition,
                   const glm::vec3 &cameraPosition) {
  if (not initialized || not toggled) {
    return;
  }

  cloudShader->use();

  { // Uniforms
    cloudShader->setMat4("projection", projection);
    cloudShader->setMat4("view", view);
  }

  { // Model matrix
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, scale);
    cloudShader->setMat4("model", model);
  }

  { // Light and camera positions
    cloudShader->setVec3("lightPos", lightPosition);
    cloudShader->setVec3("cameraPos", cameraPosition);
  }

  { // Texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, volumeTexture);
    cloudShader->setInt("volumeTexture", 0);
  }

  { // Transparency
    cloudShader->setFloat("alpha", 0.8f);
  }

  // for each slice, do render
  for (int i = 0; i < SLICE_COUNT; i++) {
    float sliceDepth =
        static_cast<float>(i) / static_cast<float>(SLICE_COUNT - 1) - 0.5f;
    cloudShader->setFloat("sliceDepth", sliceDepth);

    glBindVertexArray(sliceVAOs[i]);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
  }
}

void Cloud::setPosition(const glm::vec3 &pos) { position = pos; }

void Cloud::setScale(const glm::vec3 &scl) { scale = scl; }

void Cloud::update(float deltaTime) {
  if (not initialized or not toggled) {
    return;
  }

  animationTime += deltaTime;
}

void Cloud::cleanup() {
  if (not initialized) {
    return;
  }

  cleanupGeometry();
  cloudShader.reset();
  initialized = false;
}

bool Cloud::setupGeometry() {
  // Create slice geometry for each layer
  float vertices[] = {// positions        // texture coords
                      -0.5f, -0.5f, 0.0f, 0.0f, 0.5f, -0.5f, 1.0f, 0.0f,
                      -0.5f, 0.5f,  0.0f, 1.0f, 0.5f, 0.5f,  1.0f, 1.0f};

  for (int i = 0; i < SLICE_COUNT; i++) {
    glGenVertexArrays(1, &sliceVAOs[i]);
    glGenBuffers(1, &sliceVBOs[i]);

    glBindVertexArray(sliceVAOs[i]);
    glBindBuffer(GL_ARRAY_BUFFER, sliceVBOs[i]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Position
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(0);

    // Texture coordinate
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
  }

  return true;
}

float lerp(float t, float a, float b) { return a + t * (b - a); }

bool Cloud::generateVolumeTexture() {
  const int width = 32;
  const int height = 32;
  const int depth = 32;

  // Create 3D texture data
  std::vector<uint8_t> data(width * height * depth);

  for (int z = 0; z < depth; z++) {
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        // Normalize coordinates
        float nx = static_cast<float>(x) / width;
        float ny = static_cast<float>(y) / height;
        float nz = static_cast<float>(z) / depth;

        // Center coordinates
        float cx = nx - 0.5f;
        float cy = ny - 0.5f;
        float cz = nz - 0.5f;

        // Distance from center
        float distance = sqrt(cx * cx + cy * cy + cz * cz);

        // Create spherical cloud shape
        float shape = 1.0f - (distance * 2.0f);
        if (shape < 0.0f)
          shape = 0.0f;

        const siv::PerlinNoise perlin{42};

        // Add noise for detail
        float noiseValue =
            perlin.octave3D_01(nx * 4.0f, ny * 4.0f, nz * 4.0f, 3);
        float density = shape * noiseValue;

        // Clamp and scale
        density = std::max(0.0f, std::min(1.0f, density));
        data[z * width * height + y * width + x] = (uint8_t)(density * 255);
      }
    }
  }

  // Generate 3D texture
  glGenTextures(1, &volumeTexture);
  glBindTexture(GL_TEXTURE_3D, volumeTexture);

  glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, width, height, depth, 0, GL_RED,
               GL_UNSIGNED_BYTE, data.data());

  // Set texture parameters
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glBindTexture(GL_TEXTURE_3D, 0);

  return true;
}

void Cloud::setupShader() {
  try {
    cloudShader = std::make_unique<Shader>("shaders/cloud.vs.glsl",
                                           "shaders/cloud.fs.glsl");
  } catch (const std::exception &e) {
    std::println(std::cerr, "Failed to load cloud shaders: {}", e.what());
    throw;
  }
}

void Cloud::cleanupGeometry() {
  for (int i = 0; i < SLICE_COUNT; i++) {
    if (sliceVAOs[i] != 0) {
      glDeleteVertexArrays(1, &sliceVAOs[i]);
      sliceVAOs[i] = 0;
    }
    if (sliceVBOs[i] != 0) {
      glDeleteBuffers(1, &sliceVBOs[i]);
      sliceVBOs[i] = 0;
    }
  }

  if (volumeTexture != 0) {
    glDeleteTextures(1, &volumeTexture);
    volumeTexture = 0;
  }
}

void Cloud::processKeyboard(Movement direction, float deltaTime) {
  // Move along X or Z
  static glm::vec3 front{0.0, 0.0, -1.0};
  static glm::vec3 right{1.0, 0.0, 0.0};

  float velocity = movementSpeed * deltaTime;
  if (direction == FORWARD)
    position += front * velocity;
  if (direction == BACKWARD)
    position -= front * velocity;
  if (direction == LEFT)
    position -= right * velocity;
  if (direction == RIGHT)
    position += right * velocity;
}
