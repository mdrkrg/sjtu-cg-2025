#include <iostream>
#include <cmath>
#include <print>
#include <vector>
#include <algorithm>
#include "cloud.h"

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
    float sliceDepth = (float)i / (float)(SLICE_COUNT - 1) - 0.5f;
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

// Simple noise function (value noise)
float Cloud::noise(float x, float y, float z) {
  // Simple hash function
  int X = (int)(floor(x)) & 255;
  int Y = (int)(floor(y)) & 255;
  int Z = (int)(floor(z)) & 255;

  x -= floor(x);
  y -= floor(y);
  z -= floor(z);

  // Fade function
  float u = x * x * x * (x * (x * 6 - 15) + 10);
  float v = y * y * y * (y * (y * 6 - 15) + 10);
  float w = z * z * z * (z * (z * 6 - 15) + 10);

  // Simple gradient values (simple hash)
  float a = (X + Y * 57 + Z * 103) & 255;
  float b = (X + 1 + Y * 57 + Z * 103) & 255;
  float c = (X + (Y + 1) * 57 + Z * 103) & 255;
  float d = (X + 1 + (Y + 1) * 57 + Z * 103) & 255;
  float e = (X + Y * 57 + (Z + 1) * 103) & 255;
  float f = (X + 1 + Y * 57 + (Z + 1) * 103) & 255;
  float g = (X + (Y + 1) * 57 + (Z + 1) * 103) & 255;
  float h = (X + 1 + (Y + 1) * 57 + (Z + 1) * 103) & 255;

  // Interpolate
  float res = lerp(w, lerp(v, lerp(u, a, b), lerp(u, c, d)),
                   lerp(v, lerp(u, e, f), lerp(u, g, h)));

  return res / 255.0f;
}

float lerp(float t, float a, float b) { return a + t * (b - a); }

float Cloud::fractalNoise(float x, float y, float z, int octaves) {
  float value = 0.0f;
  float amplitude = 0.5f;
  float frequency = 1.0f;

  for (int i = 0; i < octaves; i++) {
    value += amplitude * noise(x * frequency, y * frequency, z * frequency);
    amplitude *= 0.5f;
    frequency *= 2.0f;
  }

  return value;
}

bool Cloud::generateVolumeTexture() {
  const int width = 32;
  const int height = 32;
  const int depth = 32;

  // Create 3D texture data
  std::vector<unsigned char> data(width * height * depth);

  for (int z = 0; z < depth; z++) {
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        // Normalize coordinates
        float nx = (float)x / width;
        float ny = (float)y / height;
        float nz = (float)z / depth;

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

        // Add noise for detail
        float noiseValue = fractalNoise(nx * 4.0f, ny * 4.0f, nz * 4.0f, 3);
        float density = shape * noiseValue;

        // Clamp and scale
        density = std::max(0.0f, std::min(1.0f, density));
        data[z * width * height + y * width + x] =
            (unsigned char)(density * 255);
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
    std::println(std::cerr, "Failed to load cloud shaders: {}",
                 e.what());
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
