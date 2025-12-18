#pragma once

#include "math/ray.hpp"
#include "scene/aabb.hpp"
#include <algorithm>
#include <optional>
#include <optional>
#include <tuple>
#include <limits>

namespace math {

/// Ray-AABB intersection using slab method
/// Returns min and max distance if intersection occurs,
/// otherwise returns nullopt
inline std::optional<std::tuple<float, float>>
raycastAABB(const Ray &ray, const scene::AABB &aabb,
            float maxDistance = std::numeric_limits<float>::infinity()) {
  glm::vec3 invDir = 1.0f / ray.direction();

  // Avoid division by zero
  glm::vec3 safeInvDir = invDir;
  for (int i = 0; i < 3; ++i) {
    if (std::abs(ray.direction()[i]) < 1e-6f) {
      safeInvDir[i] = std::numeric_limits<float>::infinity();
    }
  }

  glm::vec3 t1 = (aabb.min - ray.origin()) * safeInvDir;
  glm::vec3 t2 = (aabb.max - ray.origin()) * safeInvDir;

  glm::vec3 tMinVec = glm::min(t1, t2);
  glm::vec3 tMaxVec = glm::max(t1, t2);

  const auto tmin = std::max(std::max(tMinVec.x, tMinVec.y), tMinVec.z);
  const auto tmax = std::min(std::min(tMaxVec.x, tMaxVec.y), tMaxVec.z);

  if (tmin > tmax or tmax < 0.0f or tmin > maxDistance) {
    return std::nullopt;
  }

  return std::make_tuple(tmin, tmax);
}

/// Ray-sphere intersection
/// Returns min and max distance if intersection occurs, otherwise nullopt
inline std::optional<std::tuple<float, float>>
raycastSphere(const Ray &ray, const glm::vec3 &center, float radius) {
  glm::vec3 oc = ray.origin() - center;
  float a = glm::dot(ray.direction(), ray.direction());
  float b = 2.0f * glm::dot(oc, ray.direction());
  float c = glm::dot(oc, oc) - radius * radius;

  float discriminant = b * b - 4.0f * a * c;
  if (discriminant < 0.0f) {
    return std::nullopt;
  }

  float sqrtDisc = std::sqrt(discriminant);
  float tmin = (-b - sqrtDisc) / (2.0f * a);
  float tmax = (-b + sqrtDisc) / (2.0f * a);

  if (tmin > tmax)
    std::swap(tmin, tmax);

  if (tmax < 0.0f) {
    return std::nullopt; // sphere is behind ray
  }

  return std::make_tuple(tmin, tmax);
}
} // namespace math
