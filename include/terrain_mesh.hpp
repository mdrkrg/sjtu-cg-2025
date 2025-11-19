#ifndef TERRAIN_MESH_H
#define TERRAIN_MESH_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>

#include <shader.h>
#include <mesh.hpp>

#include <vector>

class TerrainMesh {
public:
  // Constructor
  TerrainMesh(int width, int height, float maxHeight);

  // Destructor
  ~TerrainMesh();

  // Render the terrain
  void render(Shader &shader);

  // Get height at a specific point
  float getHeightAt(float x, float z) const;

private:
  // Terrain data
  int width, height;
  float maxHeight;
  std::vector<float> heightData;

  // Mesh data
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

#endif
