#pragma once

#include <glm/glm.hpp>
namespace scene {

// Axis-aligned bounding box
struct AABB {
  glm::vec3 min;
  glm::vec3 max;

  AABB() : min{0.0f}, max{0.0f} {}
  AABB(const glm::vec3 &min, const glm::vec3 &max) : min{min}, max{max} {}

  /// Expand AABB to include point
  void expand(const glm::vec3 &point) {
    min = glm::min(min, point);
    max = glm::max(max, point);
  }

  /// Get center of AABB
  glm::vec3 center() const { return (min + max) * 0.5f; }

  /// Get half extents
  glm::vec3 halfExtents() const { return (max - min) * 0.5f; }

  /// Get a copied AABB transformed by a model matrix
  AABB transform(const glm::mat4 &modelMatrix) const {
    // Transform all 8 corners and compute new min/max
    glm::vec3 corners[8] = {{min.x, min.y, min.z}, {max.x, min.y, min.z},
                            {min.x, max.y, min.z}, {max.x, max.y, min.z},
                            {min.x, min.y, max.z}, {max.x, min.y, max.z},
                            {min.x, max.y, max.z}, {max.x, max.y, max.z}};

    glm::vec3 transformedMin(std::numeric_limits<float>::max());
    glm::vec3 transformedMax(std::numeric_limits<float>::lowest());

    for (const auto &corner : corners) {
      glm::vec4 transformed = modelMatrix * glm::vec4(corner, 1.0f);
      transformedMin = glm::min(transformedMin, glm::vec3(transformed));
      transformedMax = glm::max(transformedMax, glm::vec3(transformed));
    }

    return AABB(transformedMin, transformedMax);
  }

  /// Merge this AABB with another AABB
  /// @param other Other AABB to merge with
  /// @return New AABB containing both AABBs
  AABB merge(const AABB &other) const {
    if (min.x == 0.0f && min.y == 0.0f && min.z == 0.0f && max.x == 0.0f &&
        max.y == 0.0f && max.z == 0.0f) {
      return other; // This AABB is empty
    }
    if (other.min.x == 0.0f && other.min.y == 0.0f && other.min.z == 0.0f &&
        other.max.x == 0.0f && other.max.y == 0.0f && other.max.z == 0.0f) {
      return *this; // Other AABB is empty
    }
    return AABB(glm::min(min, other.min), glm::max(max, other.max));
  }
};
} // namespace scene
