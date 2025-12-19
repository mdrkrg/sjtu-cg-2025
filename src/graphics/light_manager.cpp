#include "graphics/light_manager.hpp"
#include <cstring>
#include <iostream>
#include <print>

namespace graphics {

LightManager::LightManager() {
  // Initialize lighting data with default values
  m_lightingData.viewPos = glm::vec4{0.0f, 0.0f, 0.0f, 1.0f};
  m_lightingData.numPointLights = 0;

  // Initialize point lights array with defaults
  for (size_t i = 0; i < MAX_POINT_LIGHTS; ++i) {
    m_lightingData.pointLights[i] = PointLight{};
  }
}

LightManager::~LightManager() { cleanup(); }

bool LightManager::init() {
  if (m_ubo != 0) {
    std::println("LightManager already initialized");
    return true;
  }

  if (not createUBO()) {
    std::println("Failed to create LightManager UBO");
    return false;
  }

  std::println("LightManager initialized with UBO handle: {}", m_ubo);
  return true;
}

void LightManager::cleanup() {
  if (m_ubo != 0) {
    glDeleteBuffers(1, &m_ubo);
    m_ubo = 0;
  }
  m_numActiveLights = 0;
  m_dirty = true;
}

bool LightManager::createUBO() {
  glGenBuffers(1, &m_ubo);
  if (m_ubo == 0) {
    std::println("Failed to generate UBO for LightManager");
    return false;
  }

  // Bind and allocate buffer
  glBindBuffer(GL_UNIFORM_BUFFER, m_ubo);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(LightingData), nullptr,
               GL_DYNAMIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);

  // Bind to binding point 1
  glBindBufferBase(GL_UNIFORM_BUFFER, getUBOBindingPoint(), m_ubo);

  // Check for errors
  GLenum error = glGetError();
  if (error != GL_NO_ERROR) {
    std::println(std::cerr, "OpenGL error during UBO creation: {}", error);
    glDeleteBuffers(1, &m_ubo);
    m_ubo = 0;
    return false;
  }

  return true;
}

std::optional<uint32_t> LightManager::addPointLight(const PointLight &light) {
  if (m_numActiveLights >= static_cast<int>(MAX_POINT_LIGHTS)) {
    std::println(std::cerr,
                 "Cannot add point light: maximum limit ({}) reached",
                 MAX_POINT_LIGHTS);
    return std::nullopt;
  }

  uint32_t index = m_numActiveLights;
  m_lightingData.pointLights[index] = light;
  m_numActiveLights++;
  m_lightingData.numPointLights = m_numActiveLights;
  m_dirty = true;

  std::println("Added point light at index {}", index);
  return index;
}

void LightManager::removePointLight(uint32_t index) {
  if (index < 0 or index >= m_numActiveLights) {
    std::println("Invalid point light index: {}", index);
    return;
  }

  // Shift lights down
  for (uint32_t i = index; i < m_numActiveLights - 1; ++i) {
    m_lightingData.pointLights[i] = m_lightingData.pointLights[i + 1];
  }

  // Clear last light
  m_lightingData.pointLights[m_numActiveLights - 1] = PointLight();
  m_numActiveLights--;
  m_lightingData.numPointLights = m_numActiveLights;
  m_dirty = true;

  std::println("Removed point light at index {}", index);
}

PointLight &LightManager::getPointLight(uint32_t index) {
  assert(index >= 0 and index < m_numActiveLights);
  m_dirty = true;
  return m_lightingData.pointLights[index];
}

const PointLight &LightManager::getPointLight(uint32_t index) const {
  assert(index >= 0 and index < m_numActiveLights);
  return m_lightingData.pointLights[index];
}

void LightManager::setViewPos(const glm::vec3 &position) {
  m_lightingData.viewPos = glm::vec4(position, 1.0f);
  m_dirty = true;
}

void LightManager::updateUBO() {
  if (not initialized()) {
    std::println("LightManager not initialized, cannot update UBO");
    return;
  }

  if (not m_dirty) {
    return; // No changes
  }

  glBindBuffer(GL_UNIFORM_BUFFER, m_ubo);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(LightingData), &m_lightingData);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);

  m_dirty = false;
}
} // namespace graphics
