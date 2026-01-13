#include "math/intersection.hpp"
#include <graphics/mesh.hpp>

#include <pmp/surface_mesh.h>
#include <pmp/algorithms/decimation.h>
#include <pmp/algorithms/remeshing.h>
#include <pmp/algorithms/subdivision.h>

#include <limits>
#include <iostream>

void Mesh::initializeFromVerticesIndices(std::vector<MeshVertex> &&vertices,
                                         std::vector<unsigned int> &&indices) {
  pmpMesh = std::make_unique<pmp::SurfaceMesh>();

  // Add vertex properties for custom attributes
  // NOTE: "v:point" is built-in and created automatically
  auto vnormal = pmpMesh->add_vertex_property<pmp::Normal>("v:normal");
  auto vtexcoord = pmpMesh->add_vertex_property<pmp::TexCoord>("v:texcoord");
  auto vtangent = pmpMesh->add_vertex_property<pmp::Normal>("v:tangent");
  auto vbitangent = pmpMesh->add_vertex_property<pmp::Normal>("v:bitangent");

  // Add vertices to PMP mesh with all attributes
  std::vector<pmp::Vertex> pmpVertices;
  pmpVertices.reserve(vertices.size());

  for (const auto &vertex : vertices) {
    // Add vertex to mesh
    pmp::Vertex v = pmpMesh->add_vertex(
        pmp::Point(vertex.Position.x, vertex.Position.y, vertex.Position.z));

    { // Store vertex attributes
      vnormal[v] =
          pmp::Normal(vertex.Normal.x, vertex.Normal.y, vertex.Normal.z);
      vtexcoord[v] = pmp::TexCoord(vertex.TexCoords.x, vertex.TexCoords.y);
      vtangent[v] =
          pmp::Normal(vertex.Tangent.x, vertex.Tangent.y, vertex.Tangent.z);
      vbitangent[v] = pmp::Normal(vertex.Bitangent.x, vertex.Bitangent.y,
                                  vertex.Bitangent.z);
    }

    pmpVertices.push_back(v);
  }

  // Add faces to mesh
  for (size_t i = 0; i < indices.size(); i += 3) {
    if (i + 2 >= indices.size())
      break;

    pmp::Vertex v0 = pmpVertices[indices[i]];
    pmp::Vertex v1 = pmpVertices[indices[i + 1]];
    pmp::Vertex v2 = pmpVertices[indices[i + 2]];

    pmpMesh->add_triangle(v0, v1, v2);
  }

  // Store original vertices and indices
  triangleVertices = std::move(vertices);
  triangleIndices = std::move(indices);
}

void Mesh::generateRenderData() {
  // Clear existing render data
  triangleVertices.clear();
  triangleIndices.clear();

  if (not pmpMesh or pmpMesh->n_vertices() == 0) {
    return;
  }

  // Get vertex properties from PMP mesh
  auto points = pmpMesh->get_vertex_property<pmp::Point>("v:point");
  auto normals = pmpMesh->get_vertex_property<pmp::Normal>("v:normal");
  auto texcoords = pmpMesh->get_vertex_property<pmp::TexCoord>("v:texcoord");
  auto tangents = pmpMesh->get_vertex_property<pmp::Normal>("v:tangent");
  auto bitangents = pmpMesh->get_vertex_property<pmp::Normal>("v:bitangent");

  if (not points) {
    return; // No vertex positions
  }

  // Create render vertices with attributes from vertex properties
  triangleVertices.reserve(pmpMesh->n_vertices());
  for (const auto &v : pmpMesh->vertices()) {
    MeshVertex vertex;

    pmp::Point p = points[v];
    pmp::Normal n = normals[v];
    pmp::TexCoord tc = texcoords[v];
    pmp::Normal t = tangents[v];
    pmp::Normal b = bitangents[v];

    vertex.Position = glm::vec3{p[0], p[1], p[2]};
    vertex.Normal = glm::vec3{n[0], n[1], n[2]};
    vertex.TexCoords = glm::vec2{tc[0], tc[1]};
    vertex.Tangent = glm::vec3{t[0], t[1], t[2]};
    vertex.Bitangent = glm::vec3{b[0], b[1], b[2]};

    triangleVertices.push_back(vertex);
  }

  // Create render indices by triangulating faces (for rendering and collision)
  triangleIndices.reserve(pmpMesh->n_faces() * 3);
  for (const auto &f : pmpMesh->faces()) {
    std::vector<pmp::Vertex> faceVertices;
    for (const auto &v : pmpMesh->vertices(f)) {
      faceVertices.push_back(v);
    }

    // Triangulate polygon (simple fan triangulation)
    if (faceVertices.size() >= 3) {
      for (size_t i = 1; i < faceVertices.size() - 1; ++i) {
        triangleIndices.push_back(faceVertices[0].idx());
        triangleIndices.push_back(faceVertices[i].idx());
        triangleIndices.push_back(faceVertices[i + 1].idx());
      }
    }
  }
}

void Mesh::updateRenderBuffers() {
  if (triangleVertices.empty()) {
    generateRenderData();
  }

  // Update OpenGL buffers
  setupMesh();
}

bool Mesh::containsPoint(const glm::vec3 &point,
                         const glm::mat4 &modelMatrix) const {
  if (triangleVertices.empty() or triangleIndices.empty()) {
    return false;
  }

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
  for (size_t i = 0; i < triangleIndices.size(); i += 3) {
    if (i + 2 >= triangleIndices.size())
      break;

    const MeshVertex &v0 = triangleVertices[triangleIndices[i]];
    const MeshVertex &v1 = triangleVertices[triangleIndices[i + 1]];
    const MeshVertex &v2 = triangleVertices[triangleIndices[i + 2]];

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
  if (triangleVertices.empty() or triangleIndices.empty()) {
    return std::nullopt;
  }

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
  for (size_t i = 0; i < triangleIndices.size(); i += 3) {
    if (i + 2 >= triangleIndices.size())
      break;

    const MeshVertex &v0 = triangleVertices[triangleIndices[i]];
    const MeshVertex &v1 = triangleVertices[triangleIndices[i + 1]];
    const MeshVertex &v2 = triangleVertices[triangleIndices[i + 2]];

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

scene::AABB Mesh::getLocalAABB() const {
  if (not pmpMesh or pmpMesh->n_vertices() == 0) {
    return scene::AABB{glm::vec3{0.0f}, glm::vec3{0.0f}};
  }

  auto points = pmpMesh->get_vertex_property<pmp::Point>("v:point");
  if (not points) {
    return scene::AABB{glm::vec3{0.0f}, glm::vec3{0.0f}};
  }

  // Get first vertex to initialize min/max
  pmp::Vertex first = *pmpMesh->vertices().begin();
  pmp::Point p = points[first];
  glm::vec3 min(p[0], p[1], p[2]);
  glm::vec3 max = min;

  // Find min/max across all vertices
  for (const auto &v : pmpMesh->vertices()) {
    p = points[v];
    glm::vec3 pos(p[0], p[1], p[2]);
    min = glm::min(min, pos);
    max = glm::max(max, pos);
  }

  return scene::AABB{min, max};
}

scene::AABB Mesh::getWorldAABB(const glm::mat4 &modelMatrix) const {
  scene::AABB localAABB = getLocalAABB();
  return localAABB.transform(modelMatrix);
}

Mesh Mesh::generateCollisionMesh(float quality) const {
  if (not pmpMesh or pmpMesh->n_vertices() == 0) {
    return Mesh{};
  }

  auto collisionPMPMesh = std::make_unique<pmp::SurfaceMesh>(*pmpMesh);

  // Quality: [0.0, 1.0]
  float targetVertexRatio = 0.1f + (quality * 0.9f);
  size_t targetVertices =
      static_cast<size_t>(collisionPMPMesh->n_vertices() * targetVertexRatio);

  // Ensure we have at least 4 vertices (tetrahedron minimum)
  targetVertices = std::max(targetVertices, static_cast<size_t>(4));

  try {
    // Apply decimation
    pmp::decimate(*collisionPMPMesh, targetVertices,
                  10,  // aspect ratio
                  0.0, // edge length
                  0,   // max valence
                  180, // normal deviation
                  0.0, // Hausdorff error
                  0.0, // seam threshold
                  0.0  // seam angle deviation
    );
  } catch (const std::exception &e) {
    std::println(std::cerr, "Mesh decimation failed: {}", e.what());
    // If decimation fails, use the original mesh
    // Keep the original copy
  }

  // Create new Mesh with simplified PMP mesh
  Mesh collisionMesh;
  collisionMesh.name = name + "_collision";
  collisionMesh.textures = textures;
  collisionMesh.pmpMesh = std::move(collisionPMPMesh);

  // Generate render buffers for the collision mesh
  collisionMesh.updateRenderBuffers();

  return collisionMesh;
}

void Mesh::setupMesh() {
  // Delete existing buffers if they exist
  if (VAO != 0) {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
  }

  if (triangleVertices.empty()) {
    VAO = 0;
    VBO = 0;
    EBO = 0;
    return;
  }

  // create buffers/arrays
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);
  // load data into vertex buffers
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, triangleVertices.size() * sizeof(MeshVertex),
               triangleVertices.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               triangleIndices.size() * sizeof(unsigned int),
               triangleIndices.data(), GL_STATIC_DRAW);

  // set the vertex attribute pointers
  // vertex Positions
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex),
                        (void *)0);
  // vertex normals
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex),
                        (void *)offsetof(MeshVertex, Normal));
  // vertex texture coords
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(MeshVertex),
                        (void *)offsetof(MeshVertex, TexCoords));
  // vertex tangent
  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex),
                        (void *)offsetof(MeshVertex, Tangent));
  // vertex bitangent
  glEnableVertexAttribArray(4);
  glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex),
                        (void *)offsetof(MeshVertex, Bitangent));

  glBindVertexArray(0);
}
