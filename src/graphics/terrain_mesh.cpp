#include "graphics/terrain_mesh.hpp"
#include <PerlinNoise.hpp>
#include <cmath>

TerrainMesh::TerrainMesh(int width, int height, float maxHeight)
    : width(width), height(height), maxHeight(maxHeight) {
  generateHeightMap(1.8f, 8ZU, std::rand());
  generateTerrainMesh();
  setupMesh();
}

TerrainMesh::~TerrainMesh() {
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &EBO);
}

/// Interpolate low and high smoothly.
float smoothstep(float low, float high, float x) {
  x = std::clamp((x - low) / (high - low), 0.0f, 1.0f);
  return x * x * (3 - 2 * x);
}

void TerrainMesh::generateHeightMap(float frequency, size_t octaves,
                                    size_t seed) {

  const siv::PerlinNoise perlin{seed};

  // 80% of the box is flat, other will be rounded
  const float innerBoxSize = 0.8f;

  const float falloffStart = innerBoxSize / 2.0f;
  const float falloffEnd = 0.5f; // edge
  const float falloffRange = falloffEnd - falloffStart;

  const float fx = frequency / width;
  const float fy = frequency / height;

  heightData.resize(height * width);

  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      double noiseValue =
          (perlin.octave2D(x * fx, y * fy, octaves) + 0.2) * maxHeight;

      float falloffModifier = [&] {
        // Normalize to [-0.5, 0.5]
        float nx = (float)x / width - 0.5f;
        float ny = (float)y / height - 0.5f;

        // Absolute distances from center
        float absNx = std::abs(nx);
        float absNy = std::abs(ny);

        // if distance < falloffStart, round to 0
        // else calculate distance from the edge to the box
        float dx = std::max(0.0f, absNx - falloffStart);
        float dy = std::max(0.0f, absNy - falloffStart);

        // Inner box: 0 distance
        // Edge: linear distance
        // Corner: euclidean distance (rounded square)
        float falloffDist = std::sqrt(dx * dx + dy * dy);

        float t = falloffDist / falloffRange;
        return 1.0f - smoothstep(0.0f, 1.0f, t);
      }();

      // Apply modifier
      heightData[x + y * width] = noiseValue * falloffModifier;
    }
  }
}

void TerrainMesh::generateTerrainMesh() {
  vertices.clear();
  indices.clear();

  // Generate vertices
  for (int z = 0; z < height; ++z) {
    for (int x = 0; x < width; ++x) {
      // Calculate height from height map
      int idx = (z * width + x);
      float h = (idx < heightData.size()) ? heightData[idx] : 0.0f;

      // Create vertex
      MeshVertex vertex;
      vertex.Position =
          glm::vec3((float)x / (width - 1) - 0.5f, // Normalize x to [-0.5, 0.5]
                    h,
                    (float)z / (height - 1) - 0.5f // Normalize z to [-0.5, 0.5]
          );

      // Init normal to zero (calculated later)
      vertex.Normal = glm::vec3(0.0f, 0.0f, 0.0f);

      // Texture coords
      vertex.TexCoords =
          glm::vec2((float)x / (width - 1), (float)z / (height - 1));

      // Init other attrs
      vertex.Tangent = glm::vec3(1.0f, 0.0f, 0.0f);
      vertex.Bitangent = glm::vec3(0.0f, 0.0f, 1.0f);

      vertices.push_back(vertex);
    }
  }

  // Generate indices for triangle strips
  for (int z = 0; z < height - 1; ++z) {
    for (int x = 0; x < width - 1; ++x) {
      int topLeft = (z * width) + x;
      int topRight = topLeft + 1;
      int bottomLeft = ((z + 1) * width) + x;
      int bottomRight = bottomLeft + 1;

      // First triangle
      indices.push_back(topLeft);
      indices.push_back(bottomLeft);
      indices.push_back(topRight);

      // Second triangle
      indices.push_back(topRight);
      indices.push_back(bottomLeft);
      indices.push_back(bottomRight);
    }
  }

  calculateNormals();
}

void TerrainMesh::calculateNormals() {
  // Reset normals
  for (auto &vertex : vertices) {
    vertex.Normal = glm::vec3(0.0f, 0.0f, 0.0f);
  }

  // Calculate face normals and accumulate at vertices
  for (size_t i = 0; i < indices.size(); i += 3) {
    const auto idx0 = indices[i];
    const auto idx1 = indices[i + 1];
    const auto idx2 = indices[i + 2];

    if (idx0 >= vertices.size() or idx1 >= vertices.size() or
        idx2 >= vertices.size()) {
      continue;
    }

    const auto v0 = vertices[idx0].Position;
    const auto v1 = vertices[idx1].Position;
    const auto v2 = vertices[idx2].Position;

    // normal = edge1 x edge2
    const auto edge1 = v1 - v0;
    const auto edge2 = v2 - v0;
    const auto faceNormal = glm::normalize(glm::cross(edge1, edge2));

    vertices[idx0].Normal += faceNormal;
    vertices[idx1].Normal += faceNormal;
    vertices[idx2].Normal += faceNormal;
  }

  // Normalize all vertex normals
  for (auto &vertex : vertices) {
    if (glm::length(vertex.Normal) > 0.0f) {
      vertex.Normal = glm::normalize(vertex.Normal);
    } else {
      // Fallback to upward
      vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
    }
  }
}

void TerrainMesh::setupMesh() {
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);

  // Load vertex data
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(MeshVertex),
               &vertices[0], GL_STATIC_DRAW);

  // Load index data
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
               &indices[0], GL_STATIC_DRAW);

  // Set vertex attribute pointers
  // Position
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex),
                        (void *)0);

  // Normal
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex),
                        (void *)offsetof(MeshVertex, Normal));

  // Texture coordinates
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(MeshVertex),
                        (void *)offsetof(MeshVertex, TexCoords));

  // Tangent
  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex),
                        (void *)offsetof(MeshVertex, Tangent));

  // Bitangent
  glEnableVertexAttribArray(4);
  glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex),
                        (void *)offsetof(MeshVertex, Bitangent));

  glBindVertexArray(0);
}

void TerrainMesh::render(Shader &shader) {
  glBindVertexArray(VAO);
  glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()),
                 GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}

float TerrainMesh::getHeightAt(float x, float z) const {
  // Convert normalized coords to height map indices
  int xIndex = static_cast<int>((x + 0.5f) * (width - 1));
  int zIndex = static_cast<int>((z + 0.5f) * (height - 1));

  // Check bound, return nan if out of bound
  if (xIndex < 0 or xIndex > width - 1 or zIndex < 0 or zIndex > height - 1) {
    return NAN;
  }

  // Get height from height data
  int idx = zIndex * width + xIndex;
  if (idx >= 0 && idx < heightData.size()) {
    return heightData[idx];
  }

  return NAN;
}
