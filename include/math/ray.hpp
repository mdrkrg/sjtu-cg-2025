#pragma once

#include "core/camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

namespace math {
class Ray {
public:
  Ray()
      : m_origin(0.0f),
        // Standard: point to -z
        m_direction(0.0f, 0.0f, -1.0f) {}
  Ray(const glm::vec3 &origin, const glm::vec3 &direction)
      : m_origin(origin), m_direction(glm::normalize(direction)) {}

  const glm::vec3 &origin() const { return m_origin; }
  const glm::vec3 &direction() const { return m_direction; }

  /// Get point at distance t along ray
  glm::vec3 pointAt(float t) const { return m_origin + m_direction * t; }

  /// Transform ray by matrix
  Ray transform(const glm::mat4 &matrix) const;

  /// Create ray from screen coordinates (mouse position)
  /// Requires camera projection and view matrices
  static Ray fromScreenCoords(float mouseX, float mouseY, float screenWidth,
                              float screenHeight, const glm::mat4 &projection,
                              const glm::mat4 &view);

  /// Create ray from camera position and direction
  static Ray fromCamera(const Camera &camera);

private:
  glm::vec3 m_origin;
  glm::vec3 m_direction;
};

inline Ray Ray::transform(const glm::mat4 &matrix) const {
  glm::vec4 newOrigin = matrix * glm::vec4(m_origin, 1.0f);
  glm::vec4 newDirection = matrix * glm::vec4(m_direction, 0.0f);
  return Ray(glm::vec3(newOrigin), glm::vec3(newDirection));
}

inline Ray Ray::fromScreenCoords(float mouseX, float mouseY, float screenWidth,
                                 float screenHeight,
                                 const glm::mat4 &projection,
                                 const glm::mat4 &view) {
  // Convert mouse coordinates to normalized device coordinates (NDC)
  float x = (2.0f * mouseX) / screenWidth - 1.0f;
  float y = 1.0f - (2.0f * mouseY) / screenHeight; // Flip Y

  // Create ray in clip space
  glm::vec4 rayClip = glm::vec4(x, y, -1.0f, 1.0f);

  // Convert to eye space
  glm::mat4 invProjection = glm::inverse(projection);
  glm::vec4 rayEye = invProjection * rayClip;
  rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

  // Convert to world space
  glm::mat4 invView = glm::inverse(view);
  glm::vec4 rayWorld = invView * rayEye;
  glm::vec3 rayDirection = glm::normalize(glm::vec3(rayWorld));

  // Ray origin is camera position (extracted from view matrix)
  // Camera position is the inverse of view translation: invView[3]
  glm::vec3 rayOrigin = glm::vec3(invView[3]);

  return Ray(rayOrigin, rayDirection);
}

inline Ray Ray::fromCamera(const Camera &camera) {
  glm::vec3 rayDirection = camera.Front;
  glm::vec3 rayOrigin = camera.Position;

  return Ray(rayOrigin, rayDirection);
}
} // namespace math
