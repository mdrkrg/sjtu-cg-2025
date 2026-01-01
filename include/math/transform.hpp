#pragma once
#include <glm/glm.hpp>

namespace math {

inline glm::vec3 transformVecToLocal(const glm::vec3 &vec,
                                     const glm::mat3 &model) {
  // For vectors, use inverse transpose of rotation part
  const auto rotationMatrix = glm::mat3(model);
  const auto inverseRotation = glm::inverse(rotationMatrix);
  const auto localVec = inverseRotation * vec;
  return localVec;
}

/// Convert world position to screen coordinates (0-1)
inline glm::vec2 worldPosToScreen(const glm::mat4 &projection,
                                  const glm::mat4 &view,
                                  const glm::vec3 &position) {
  const auto clipPos = projection * view * glm::vec4{position, 1.0f};
  const auto ndc = glm::vec3{clipPos} / clipPos.w;
  auto screenPos = (glm::vec2{ndc} + 1.0f) * 0.5f;
  screenPos = glm::clamp(screenPos, 0.0f, 1.0f);
  return screenPos;
}
} // namespace math
