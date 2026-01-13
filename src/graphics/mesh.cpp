#include "math/intersection.hpp"
#include <graphics/mesh.hpp>

#include <limits>

bool Mesh::containsPoint(const glm::vec3 &point,
                         const glm::mat4 &modelMatrix) const {
  // Transform point to mesh local space
  glm::mat4 invModelMatrix = inverse(modelMatrix);
  glm::vec3 localPoint = glm::vec3(invModelMatrix * glm::vec4(point, 1.0f));

  // Use ray casting parity test: shoot a ray in +X direction
  // Count intersections with mesh triangles
  // Odd number of intersections = point is inside

  const math::Ray localRay{
      localPoint,                 // origin
      glm::vec3(1.0f, 0.0f, 0.0f) // +X direction
  };

  int intersectionCount = 0;

  // Iterate through all triangles
  for (size_t i = 0; i < indices.size(); i += 3) {
    if (i + 2 >= indices.size())
      break;

    const Vertex &v0 = vertices[indices[i]];
    const Vertex &v1 = vertices[indices[i + 1]];
    const Vertex &v2 = vertices[indices[i + 2]];

    auto hit = math::rayTriangleIntersection(localRay, v0.Position, v1.Position,
                                             v2.Position);

    if (hit && hit->distance > 0.0f) {
      intersectionCount++;
    }
  }

  // Point is inside if we have an odd number of intersections
  return (intersectionCount % 2) == 1;
}

std::optional<math::RayHit>
Mesh::rayIntersection(const math::Ray &ray,
                      const glm::mat4 &modelMatrix) const {

  const glm::mat4 invModelMatrix = inverse(modelMatrix);

  const math::Ray localRay = [&invModelMatrix, &ray] {
    // Transform ray to mesh local space
    glm::vec3 localOrigin =
        glm::vec3(invModelMatrix * glm::vec4(ray.origin(), 1.0f));
    glm::vec3 localDirection =
        normalize(glm::vec3(invModelMatrix * glm::vec4(ray.direction(), 0.0f)));
    return math::Ray{localOrigin, localDirection};
  }();

  std::optional<math::RayHit> closestHit = std::nullopt;
  float closestDistance = std::numeric_limits<float>::max();

  // Iterate through all triangles
  for (size_t i = 0; i < indices.size(); i += 3) {
    if (i + 2 >= indices.size())
      break;

    const Vertex &v0 = vertices[indices[i]];
    const Vertex &v1 = vertices[indices[i + 1]];
    const Vertex &v2 = vertices[indices[i + 2]];

    auto hit = math::rayTriangleIntersection(localRay, v0.Position, v1.Position,
                                             v2.Position);

    if (hit && hit->distance < closestDistance && hit->distance > 0.0f) {
      closestHit = hit;
      closestDistance = hit->distance;
      closestHit->triangleIdx = static_cast<unsigned int>(i / 3);

      // Transform hit back to world space
      closestHit->point = glm::vec3(modelMatrix * glm::vec4(hit->point, 1.0f));
      closestHit->normal = normalize(
          glm::vec3(transpose(invModelMatrix) * glm::vec4(hit->normal, 0.0f)));
    }
  }

  return closestHit;
}
