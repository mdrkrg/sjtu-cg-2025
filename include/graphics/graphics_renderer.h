#pragma once

#include <epoxy/gl.h>
#include <epoxy/glx.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>

#include "graphics/particles/particle_system.hpp"
#include "particles/model_particle_system.hpp"
#include "particles/arrow_launcher.hpp"
#include "shader.h"
#include "terrain_mesh.hpp"
#include "cloud.h"
#include "scene/game_object.hpp"
#include "graphics/light_manager.hpp"
#include "graphics/postprocessing/post_processing_manager.hpp"

using graphics::particles::ParticleSystem;
using graphics::postprocessing::PostProcessingManager;

class GameManager;

class GraphicsRenderer {
public:
  GraphicsRenderer(std::shared_ptr<GameManager> gameManager);
  ~GraphicsRenderer();

  bool initialize();
  void render(const glm::mat4 &projection, const glm::mat4 &view,
              const glm::vec3 &cameraPosition);
  void renderDirect(const glm::mat4 &projection, const glm::mat4 &view,
                    const glm::vec3 &cameraPosition);
  void update(float deltaTime);
  void cleanup();
  void togglePause() { paused = not paused; }

  // Pass to the input handler
  std::shared_ptr<Cloud> getCloud() { return cloud; }
  std::shared_ptr<ParticleSystem<Particle>> getRainSystem() {
    return rainSystem;
  }
  std::shared_ptr<ParticleSystem<Particle>> getSnowSystem() {
    return snowSystem;
  }
  void activateOrbAura(GameObject *const parent);

  // Mouse interaction
  void handleMouseClick(const math::Ray &ray);
  GameObject *getSelectedObject() const { return selectedObject; }

  void toggleDebug() { debugAABBsEnabled = not debugAABBsEnabled; }

  // Arrow launcher access
  std::shared_ptr<graphics::particles::ArrowLauncher> getArrowLauncher() const {
    return arrowLauncher;
  }

private:
  /// Whether the update is paused
  bool paused = false;
  // Shaders
  std::shared_ptr<Shader> lightingShader;
  std::shared_ptr<Shader> modelShader;
  std::shared_ptr<Shader> modelSimpleShader;
  std::shared_ptr<Shader> modelSimpleInstancedShader;
  std::shared_ptr<Shader> modelInstancedShader;
  std::shared_ptr<Shader> windowShader;
  std::shared_ptr<Shader> particleShader;
  std::shared_ptr<Shader> debugShader;

  // Lighting management
  graphics::LightManager lightManager;

  // TODO: Normally we would use an object manager to handle position stuff
  // For simplicity we define it here

  constexpr static glm::vec3 lightPosition{0.0f, 0.75f, 1.65f};
  const static glm::vec3 weatherPosition;
  const static glm::mat4 terrainModel;

  // Textures
  unsigned int windowDiffuseMap;
  Texture floorTexture{};
  Texture wallTexture{};

  std::shared_ptr<GameManager> gameManager;

  // Game objects (pointers for shader selection)
  GameObject *selectedObject;

  // Post-processing manager
  std::unique_ptr<PostProcessingManager> postProcessingManager;

  std::shared_ptr<TerrainMesh> terrainMesh;

  std::shared_ptr<ParticleSystem<Particle>> rainSystem;
  std::shared_ptr<ParticleSystem<Particle>> snowSystem;
  std::shared_ptr<ParticleSystem<Particle>> orbAuraSystem;

  std::shared_ptr<graphics::particles::ArrowLauncher> arrowLauncher;

  // Cloud
  std::shared_ptr<Cloud> cloud;

  // Light cube GameObject
  std::shared_ptr<GameObject> lightCubeObject;

  // Vertex data for room geometry
  struct RoomGeometry {
    unsigned int VAO;
    unsigned int VBO;
    unsigned int vertexCount;
  };

  RoomGeometry debugCubeLines;

  bool debugAABBsEnabled = false;

  // Vertex data
  static constexpr size_t VERTEX_SIZE = 8;
  static constexpr size_t VERTEX_COUNT = 48;

  // Private methods
  bool setupGeometry();
  bool setupRoomGameObjects();
  bool setupShaders();
  bool loadTextures();
  bool loadModels();

  void preRender(const glm::mat4 &projection, const glm::mat4 &view,
                 const glm::vec3 &cameraPosition);

  void setupTrap();
  void setupPuzzle();
  void setupFloorCompartment();

  void setupGeometryComponent(RoomGeometry &geometry, const float *vertices,
                              size_t vertexCount);
  void cleanupGeometryComponent(RoomGeometry &geometry);

  void renderObjects(const glm::mat4 &projection, const glm::mat4 &view);
  void renderLightCube(const glm::mat4 &projection, const glm::mat4 &view);
  void renderTerrain(const glm::mat4 &projection, const glm::mat4 &view,
                     const glm::vec3 &cameraPosition);
  void renderParticles(const glm::mat4 &projection, const glm::mat4 &view,
                       const glm::vec3 &cameraPosition);
  void renderDebugAABBs(const glm::mat4 &projection, const glm::mat4 &view);

  /// Render an individual AABB for debugging
  void renderDebugAABB(const scene::AABB &aabb) const;
};
