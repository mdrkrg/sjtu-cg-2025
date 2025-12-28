#pragma once

#include <glm/glm.hpp>
#include <algorithm>

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

  /// Check if this AABB overlaps with another AABB
  /// @param other Other AABB to check
  /// @param margin Optional margin around AABBs for collision detection
  /// @return True if AABBs overlap (with margin)
  bool overlaps(const AABB &other, float margin = 0.0f) const {
    const bool outOfBound = (max.x + margin < other.min.x - margin or
                             min.x - margin > other.max.x + margin or
                             max.y + margin < other.min.y - margin or
                             min.y - margin > other.max.y + margin or
                             max.z + margin < other.min.z - margin or
                             min.z - margin > other.max.z + margin);
    return not outOfBound;
  }

  /// Check if this AABB wraps a point (Point-in-AABB test)
  /// @param point The point to check
  /// @param margin Optional margin around AABBs for collision detection
  /// @return True if point in AABB (with margin)
  bool wraps(const glm::vec3 &point, float margin = 0.0f) const {
    const bool outOfBound = (max.x + margin < point.x - margin or
                             min.x - margin > point.x + margin or
                             max.y + margin < point.y - margin or
                             min.y - margin > point.y + margin or
                             max.z + margin < point.z - margin or
                             min.z - margin > point.z + margin);
    return not outOfBound;
  }

  /// Calculate approximate normal from AABB to point, for collision
  /// @param point Collision point in world space
  /// @return Approximate collision normal
  glm::vec3 closestNormalFrom(const glm::vec3 &point) {
    // Calculate distance from arrow to each AABB face
    float distToMinX = std::abs(point.x - min.x);
    float distToMaxX = std::abs(point.x - max.x);
    float distToMinY = std::abs(point.y - min.y);
    float distToMaxY = std::abs(point.y - max.y);
    float distToMinZ = std::abs(point.z - min.z);
    float distToMaxZ = std::abs(point.z - max.z);

    // Find closest face
    float minDist = std::min({distToMinX, distToMaxX, distToMinY, distToMaxY,
                              distToMinZ, distToMaxZ});

    // Return normal pointing away from closest face
    if (minDist == distToMinX) {
      return glm::vec3{-1.0f, 0.0f, 0.0f}; // Left
    } else if (minDist == distToMaxX) {
      return glm::vec3{1.0f, 0.0f, 0.0f}; // Right
    } else if (minDist == distToMinY) {
      return glm::vec3{0.0f, -1.0f, 0.0f}; // Bottom
    } else if (minDist == distToMaxY) {
      return glm::vec3{0.0f, 1.0f, 0.0f}; // Top
    } else if (minDist == distToMinZ) {
      return glm::vec3{0.0f, 0.0f, -1.0f}; // Front
    } else {
      // minDist == distToMaxZ
      return glm::vec3{0.0f, 0.0f, 1.0f}; // Back
    }
  }
};
} // namespace scene
