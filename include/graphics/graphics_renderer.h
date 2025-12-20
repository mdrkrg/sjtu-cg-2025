#pragma once

#include <epoxy/gl.h>
#include <epoxy/glx.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>

#include "particles/particle_system.hpp"
#include "shader.h"
#include "terrain_mesh.hpp"
#include "cloud.h"
#include "scene/game_object.hpp"
#include "graphics/light_manager.hpp"

class GameManager;

class GraphicsRenderer {
public:
  GraphicsRenderer(std::shared_ptr<GameManager> gameManager);
  ~GraphicsRenderer();

  bool initialize();
  void render(const glm::mat4 &projection, const glm::mat4 &view,
              const glm::vec3 &cameraPosition);
  void update(float deltaTime);
  void cleanup();

  // Pass to the input handler
  std::shared_ptr<Cloud> getCloud() { return cloud; }
  std::shared_ptr<ParticleSystem> getRainSystem() { return rainSystem; }
  std::shared_ptr<ParticleSystem> getSnowSystem() { return snowSystem; }

  // Mouse interaction
  void handleMouseClick(const glm::vec3 &rayOrigin, const glm::vec3 &rayDir);
  GameObject *getSelectedObject() const { return selectedObject; }

  void toggleDebug() { debugAABBsEnabled = not debugAABBsEnabled; }

private:
  // Shaders
  std::shared_ptr<Shader> lightingShader;
  std::shared_ptr<Shader> modelShader;
  std::shared_ptr<Shader> modelSimpleShader;
  std::shared_ptr<Shader> lightCubeShader;
  std::shared_ptr<Shader> windowShader;
  std::shared_ptr<Shader> particleShader;
  std::shared_ptr<Shader> debugShader;

  // Lighting management
  graphics::LightManager lightManager;

  // TODO: Normally we would use an object manager to handle position stuff
  // For simplicity we define it here

  constexpr static glm::vec3 lightPosition{0.0f, 0.75f, 1.65f};
  const static glm::mat4 weatherModel;
  const static glm::vec3 weatherPosition;
  const static glm::mat4 terrainModel;

  // Textures
  unsigned int windowDiffuseMap;

  std::shared_ptr<GameManager> gameManager;

  // Game objects (pointers for shader selection)
  GameObject *selectedObject;

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
  RoomGeometry debugCubeLines;

  bool debugAABBsEnabled = false;

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
                  const glm::vec3 &cameraPosition);
  void renderObjects(const glm::mat4 &projection, const glm::mat4 &view);
  void renderLightCube(const glm::mat4 &projection, const glm::mat4 &view);
  void renderTerrain(const glm::mat4 &projection, const glm::mat4 &view,
                     const glm::vec3 &cameraPosition);
  void renderParticles(const glm::mat4 &projection, const glm::mat4 &view,
                       const glm::vec3 &cameraPosition);
  void renderDebugAABBs(const glm::mat4 &projection, const glm::mat4 &view);
};
