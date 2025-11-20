#include "terrain_mesh.hpp"
#include <PerlinNoise.hpp>
#include <cmath>
#include <cstdlib>

TerrainMesh::TerrainMesh(int width, int height, float maxHeight)
    : width(width), height(height), maxHeight(maxHeight) {

  // Generate height map data
  generateHeightMap(1.8f, 8ZU, std::rand());

  // Generate terrain mesh
  generateTerrainMesh();

  // Setup OpenGL buffers
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
      float falloffModifier = 1.0f - smoothstep(0.0f, 1.0f, t);

      // Apply modifier
      double noiseValue =
          (perlin.octave2D(x * fx, y * fy, octaves) + 0.2) * maxHeight;
      noiseValue *= falloffModifier;

      heightData[x + y * width] = noiseValue;
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
      Vertex vertex;
      vertex.Position =
          glm::vec3((float)x / (width - 1) - 0.5f, // Normalize x to [-0.5, 0.5]
                    h,
                    (float)z / (height - 1) - 0.5f // Normalize z to [-0.5, 0.5]
          );

      // Initialize normal to zero (calculated later)
      vertex.Normal = glm::vec3(0.0f, 0.0f, 0.0f);

      // Texture coordinates
      vertex.TexCoords =
          glm::vec2((float)x / (width - 1), (float)z / (height - 1));

      // Initialize other attributes
      vertex.Tangent = glm::vec3(1.0f, 0.0f, 0.0f);
      vertex.Bitangent = glm::vec3(0.0f, 0.0f, 1.0f);

      // Initialize bone data
      for (int i = 0; i < MAX_BONE_INFLUENCE; ++i) {
        vertex.m_BoneIDs[i] = 0;
        vertex.m_Weights[i] = 0.0f;
      }

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

  // Calculate normals from geometry
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

    // Cross product to calculate normal of the face
    const auto edge1 = v1 - v0;
    const auto edge2 = v2 - v0;
    const auto faceNormal = glm::normalize(glm::cross(edge1, edge2));

    // Accumulate face normal to each vertex
    vertices[idx0].Normal += faceNormal;
    vertices[idx1].Normal += faceNormal;
    vertices[idx2].Normal += faceNormal;
  }

  // Normalize all vertex normals
  for (auto &vertex : vertices) {
    if (glm::length(vertex.Normal) > 0.0f) {
      vertex.Normal = glm::normalize(vertex.Normal);
    } else {
      // Fallback to upward normal if no faces contributed
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
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0],
               GL_STATIC_DRAW);

  // Load index data
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
               &indices[0], GL_STATIC_DRAW);

  // Set vertex attribute pointers
  // Position
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);

  // Normal
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, Normal));

  // Texture coordinates
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, TexCoords));

  // Tangent
  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, Tangent));

  // Bitangent
  glEnableVertexAttribArray(4);
  glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, Bitangent));

  // Bone IDs
  glEnableVertexAttribArray(5);
  glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex),
                         (void *)offsetof(Vertex, m_BoneIDs));

  // Weights
  glEnableVertexAttribArray(6);
  glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, m_Weights));

  glBindVertexArray(0);
}

void TerrainMesh::render(Shader &shader) {
  glBindVertexArray(VAO);
  glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()),
                 GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}

/// Returns nan when out of bound
float TerrainMesh::getHeightAt(float x, float z) const {
  // Convert normalized coordinates to height map indices
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
