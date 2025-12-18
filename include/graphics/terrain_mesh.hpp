#pragma once

#include <graphics/shader.h>
#include <graphics/mesh.hpp>

#include <vector>

class TerrainMesh {
public:
  TerrainMesh(int width, int height, float maxHeight);
  ~TerrainMesh();

  /// Render the terrain
  void render(Shader &shader);

  /// Get height at a specific point, returns nan when out of bound
  float getHeightAt(float x, float z) const;

private:
  int width, height;
  float maxHeight;
  std::vector<float> heightData;

  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  std::vector<Texture> textures;

  // OpenGL buffers
  unsigned int VAO, VBO, EBO;

  /// Generate height map data
  void generateHeightMap(float frequency, size_t octaves, size_t seed);

  /// Generate terrain mesh
  void generateTerrainMesh();

  /// Calculate normals from geometry
  void calculateNormals();

  /// Setup OpenGL buffers
  void setupMesh();
};
