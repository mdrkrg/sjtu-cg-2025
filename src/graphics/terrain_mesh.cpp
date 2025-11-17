#include "terrain_mesh.hpp"
#include <iostream>
#include <cmath>

TerrainMesh::TerrainMesh(const std::string &heightMapPath, float maxHeight)
    : maxHeight(maxHeight), heightMapPath(heightMapPath) {

  // Load height map data
  if (!loadHeightMap(heightMapPath)) {
    std::cerr << "Failed to load height map: " << heightMapPath << std::endl;
    return;
  }

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

bool TerrainMesh::loadHeightMap(const std::string &path) {
  int imgWidth, imgHeight, channels;
  unsigned char *data =
      stbi_load(path.c_str(), &imgWidth, &imgHeight, &channels, 1);
  height = imgHeight;
  width = imgWidth;

  if (!data) {
    return false;
  }

  // Resize height data
  heightData.resize(imgWidth * imgHeight);

  // Convert image data to height values
  for (int i = 0; i < imgWidth * imgHeight; ++i) {
    const auto height = (float)data[i] / 255.0f * maxHeight;
    heightData[i] = height;
  }

  stbi_image_free(data);
  return true;
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

      // Simple normal calculation (for better results, you could calculate from
      // neighboring vertices)
      vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);

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

void TerrainMesh::Draw(Shader &shader) {
  // For now, we don't need textures for the terrain
  glBindVertexArray(VAO);
  glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()),
                 GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}

float TerrainMesh::getHeightAt(float x, float z) const {
  // Convert normalized coordinates to height map indices
  int xIndex = static_cast<int>((x + 0.5f) * (width - 1));
  int zIndex = static_cast<int>((z + 0.5f) * (height - 1));

  // Clamp to valid range
  xIndex = std::max(0, std::min(width - 1, xIndex));
  zIndex = std::max(0, std::min(height - 1, zIndex));

  // Get height from height data
  int idx = zIndex * width + xIndex;
  if (idx >= 0 && idx < heightData.size()) {
    return heightData[idx];
  }

  return 0.0f;
}
