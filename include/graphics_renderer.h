#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>

#include "particles/particle_system.hpp"
#include "shader.h"
#include "model.hpp"
#include "terrain_mesh.hpp"
#include "cloud.h"

class GraphicsRenderer {
public:
  GraphicsRenderer();
  ~GraphicsRenderer();

  bool initialize();
  void render(const glm::mat4 &projection, const glm::mat4 &view,
              const glm::vec3 &cameraPosition, const glm::vec3 &lightPosition);
  void update(float deltaTime);
  void cleanup();

  // Pass to the input handler
  std::shared_ptr<Cloud> getCloud() { return cloud; }
  std::shared_ptr<ParticleSystem> getRainSystem() { return rainSystem; }
  std::shared_ptr<ParticleSystem> getSnowSystem() { return snowSystem; }

private:
  // Shaders
  std::unique_ptr<Shader> lightingShader;
  std::unique_ptr<Shader> modelShader;
  std::unique_ptr<Shader> lightCubeShader;
  std::unique_ptr<Shader> windowShader;
  std::unique_ptr<Shader> particleShader;

  // Textures
  unsigned int windowDiffuseMap;

  // Models
  std::unique_ptr<Model> tableModel;

  std::shared_ptr<TerrainMesh> terrainMesh;

  std::shared_ptr<ParticleSystem> rainSystem;
  std::shared_ptr<ParticleSystem> snowSystem;

  // Cloud
  std::shared_ptr<Cloud> cloud;

  // Vertex data for room geometry
  struct RoomGeometry {
    unsigned int VAO;
    unsigned int VBO;
    unsigned int vertexCount;
  };

  RoomGeometry ceiling;
  RoomGeometry floor;
  RoomGeometry leftWall;
  RoomGeometry rightWall;
  RoomGeometry frontWall;
  RoomGeometry lightCube;

  // Vertex data
  static constexpr size_t VERTEX_SIZE = 8;
  static constexpr size_t VERTEX_COUNT = 48;

  // Private methods
  bool setupRoomGeometry();
  bool setupShaders();
  bool loadTextures();
  bool loadModels();

  void setupGeometryComponent(RoomGeometry &geometry, const float *vertices,
                              size_t vertexCount);
  void cleanupGeometryComponent(RoomGeometry &geometry);

  void renderRoom(const glm::mat4 &projection, const glm::mat4 &view,
                  const glm::vec3 &cameraPosition,
                  const glm::vec3 &lightPosition);
  void renderTable(const glm::mat4 &projection, const glm::mat4 &view,
                   const glm::vec3 &cameraPosition,
                   const glm::vec3 &lightPosition);
  void renderLightCube(const glm::mat4 &projection, const glm::mat4 &view,
                       const glm::vec3 &lightPosition);
  void renderTerrain(const glm::mat4 &projection, const glm::mat4 &view,
                     const glm::vec3 &cameraPosition,
                     const glm::vec3 &lightPosition);
  void renderParticles(const glm::mat4 &projection, const glm::mat4 &view,
                       const glm::vec3 &cameraPosition,
                       const glm::vec3 &lightPosition);
};
