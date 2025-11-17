#ifndef TERRAIN_MESH_H
#define TERRAIN_MESH_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>

#include <shader.h>
#include <mesh.hpp>

#include <string>
#include <vector>

class TerrainMesh {
public:
  // Constructor
  TerrainMesh(const std::string &heightMapPath, float maxHeight);

  // Destructor
  ~TerrainMesh();

  // Render the terrain
  void Draw(Shader &shader);

  // Get height at a specific point
  float getHeightAt(float x, float z) const;

  // Get terrain dimensions
  int getWidth() const { return width; }
  int getHeight() const { return height; }
  float getMaxHeight() const { return maxHeight; }

private:
  // Terrain data
  int width, height;
  float maxHeight;
  std::vector<float> heightData;
  std::string heightMapPath;

  // Mesh data
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  std::vector<Texture> textures;

  // OpenGL buffers
  unsigned int VAO, VBO, EBO;

  // Load height map data
  bool loadHeightMap(const std::string &path);

  // Generate terrain mesh
  void generateTerrainMesh();

  // Setup OpenGL buffers
  void setupMesh();

  // Normalize coordinates
  float normalizeCoordinate(float coord, int maxCoord) const;
};

#endif
