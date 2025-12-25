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
} // namespace math
