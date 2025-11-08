#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>

#include "shader.h"
#include "model.hpp"

class GraphicsRenderer {
public:
  GraphicsRenderer();
  ~GraphicsRenderer();

  bool initialize();
  void render(const glm::mat4 &projection, const glm::mat4 &view,
              const glm::vec3 &cameraPosition, const glm::vec3 &lightPosition);
  void cleanup();

private:
  // Shaders
  std::unique_ptr<Shader> lightingShader;
  std::unique_ptr<Shader> modelShader;
  std::unique_ptr<Shader> lightCubeShader;
  std::unique_ptr<Shader> windowShader;

  // Textures
  unsigned int windowDiffuseMap;

  // Models
  std::unique_ptr<Model> tableModel;

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
};
