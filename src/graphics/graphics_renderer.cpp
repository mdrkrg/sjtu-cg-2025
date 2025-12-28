#include "graphics/graphics_renderer.h"
#include <GLFW/glfw3.h>
#include "graphics/particles/particle_factory.hpp"
#include "graphics/particles/model_particle_factory.hpp"
#include "graphics/model_factory.hpp"
#include "graphics/texture.hpp"
#include "scene/behaviours/lamp_lighting_behaviour.hpp"
#include "scene/behaviours/puzzle_movement_behaviour.hpp"
#include "scene/behaviours/hidden_cell_behaviour.hpp"
#include "scene/behaviours/trap_trigger_behaviour.hpp"
#include "scene/behaviours/trap_behaviour.hpp"
#include "scene/behaviours/wall_panel_behaviour.hpp"
#include "scene/behaviours/floor_compartment_behaviour.hpp"
#include "scene/behaviours/orb_glow_behaviour.hpp"
#include "scene/utils.hpp"
#include "scene/game_object.hpp"
#include "scene/game_manager.hpp"
#include <filesystem>
#include <iostream>
#include <cstring>
#include <memory>

const glm::mat4 GraphicsRenderer::weatherModel =
    glm::scale(glm::mat4(1.0), glm::vec3(0.04));

const glm::vec3 GraphicsRenderer::weatherPosition = {0.0f, 0.0f, 0.0f};

// Scale terrain to fit on the table
const glm::mat4 GraphicsRenderer::terrainModel = glm::scale(
    // Position terrain on the table
    glm::translate(glm::mat4(1.0f), glm::vec3(0.15f, 0.15f, 2.0f)),
    glm::vec3(0.24f));

GraphicsRenderer::GraphicsRenderer(std::shared_ptr<GameManager> gameManager)
    : lightingShader(nullptr), modelShader(nullptr), modelSimpleShader(nullptr),
      modelSimpleInstancedShader(nullptr), lightCubeShader(nullptr),
      windowShader(nullptr), particleShader(nullptr), debugShader(nullptr),
      windowDiffuseMap(0), gameManager(gameManager), selectedObject(nullptr),
      terrainMesh(std::make_shared<TerrainMesh>(1024, 1024, 0.2f)),
      orbAuraSystem(nullptr), testCubeSystem(nullptr), cloud(nullptr) {
  // Initialize geometry components
  ceiling = {0, 0, 0};
  floor = {0, 0, 0};
  leftWall = {0, 0, 0};
  rightWall = {0, 0, 0};
  frontWall = {0, 0, 0};
  lightCube = {0, 0, 0};
  debugCubeLines = {0, 0, 0};
}

GraphicsRenderer::~GraphicsRenderer() { cleanup(); }

void GraphicsRenderer::activateOrbAura(GameObject *const parent) {
  orbAuraSystem = ParticleFactory::createOrbAuraSystem(
      parent, particleShader, glm::vec4(0.2f, 0.8f, 1.0f, 0.6f), 1.0f,
      1000000Z);
  orbAuraSystem->toggle(true);
}

bool GraphicsRenderer::initialize() {
  if (!setupShaders()) {
    std::cout << "Failed to setup shaders" << std::endl;
    return false;
  }

  // Init LightManager and UBO
  if (!lightManager.init()) {
    std::println(std::cerr, "Failed to initialize LightManager");
    return false;
  }

  // Default room light
  graphics::PointLight roomLight = [] {
    glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
    glm::vec3 lightAmbient = lightColor * glm::vec3(0.2f);
    glm::vec3 lightDiffuse = lightColor * glm::vec3(0.5f);
    glm::vec3 lightSpecular(0.5f, 0.5f, 0.5f);

    float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.032f;
    return graphics::PointLight(lightPosition, lightAmbient, lightDiffuse,
                                lightSpecular, constant, linear, quadratic);
  }();

  const auto lightIndex = lightManager.addPointLight(roomLight);
  if (not lightIndex.has_value()) {
    std::cout << "Failed to add default room light" << std::endl;
    return false;
  }

  std::cout << "Added default room light at index " << lightIndex.value()
            << std::endl;

  if (!loadTextures()) {
    std::cout << "Failed to load textures" << std::endl;
    return false;
  }

  if (!loadModels()) {
    std::cout << "Failed to load models" << std::endl;
    return false;
  }

  if (!setupRoomGeometry()) {
    std::cout << "Failed to setup room geometry" << std::endl;
    return false;
  }

  // Initialize cloud
  cloud = std::make_shared<Cloud>();
  if (not cloud->initialize()) {
    std::println(std::cerr, "Failed to initialize volumetric cloud");
    return false;
  }

  // Position the cloud above the table
  cloud->setPosition(glm::vec3(0.0f, 0.65f, 2.0f));
  cloud->setScale(glm::vec3(2.0f, 0.5f, 2.0f));

  rainSystem = ParticleFactory::createRainSystem(
      terrainMesh, terrainModel, particleShader, weatherPosition, 100000, 500.f,
      cloud->getGameObject());
  snowSystem = ParticleFactory::createSnowSystem(
      terrainMesh, terrainModel, particleShader, weatherPosition, 1000000,
      500.f, cloud->getGameObject());

  // Create a simple test particle system with cube model
  testCubeSystem =
      graphics::particles::ModelParticleFactory::createCubeTestSystem(
          modelSimpleInstancedShader,
          glm::vec3{0.0f, 0.5f, 2.0f}, // position above table
          50,                          // max particles
          2.0f,                        // emission rate (2 cubes/sec)
          0.1f,                        // cube size
          glm::vec3{1.0f, 0.0f, 0.0f}, // red color
          gameManager->getObjects()    // AABB collision targets
      );

  return true;
}

void GraphicsRenderer::render(const glm::mat4 &projection,
                              const glm::mat4 &view,
                              const glm::vec3 &cameraPosition) {
  // Update LightManager with camera position
  lightManager.setViewPos(cameraPosition);
  // Send data to to GPU
  lightManager.updateUBO();

  renderRoom(projection, view, cameraPosition);
  renderObjects(projection, view);
  renderLightCube(projection, view);
  // Render cloud
  if (cloud && cloud->isInitialized()) {
    cloud->render(projection, view, lightPosition, cameraPosition);
  }
  renderTerrain(projection, view, cameraPosition);
  renderParticles(projection, view, cameraPosition);
  renderDebugAABBs(projection, view);
}

void GraphicsRenderer::update(float deltaTime) {
  // Update game objects (animations) via GameManager
  gameManager->update(deltaTime);

  // Update cloud
  if (cloud && cloud->isInitialized()) {
    cloud->update(deltaTime);
  }

  // Update test cube system
  if (testCubeSystem) {
    testCubeSystem->update(deltaTime);
  }
}

void GraphicsRenderer::handleMouseClick(const glm::vec3 &rayOrigin,
                                        const glm::vec3 &rayDir) {
  // Use GameManager for ray casting and selection
  GameObject *selected = gameManager->handleRayCast(rayOrigin, rayDir);
  if (selected) {
    std::cout << "Selected object: " << selected->getName() << std::endl;
    // TODO: Trigger object-specific behavior (e.g., lamp toggle)
  } else {
    std::cout << "No object selected" << std::endl;
  }
  // Update local selectedObject pointer for consistency
  selectedObject = gameManager->getSelectedObject();
}

void GraphicsRenderer::cleanup() {
  // Cleanup geometry
  cleanupGeometryComponent(ceiling);
  cleanupGeometryComponent(floor);
  cleanupGeometryComponent(leftWall);
  cleanupGeometryComponent(rightWall);
  cleanupGeometryComponent(frontWall);
  cleanupGeometryComponent(lightCube);
  cleanupGeometryComponent(debugCubeLines);

  // Cleanup shaders
  lightingShader.reset();
  modelShader.reset();
  lightCubeShader.reset();
  windowShader.reset();
  particleShader.reset();
  debugShader.reset();

  // Cleanup game objects
  terrainMesh.reset();

  // Cleanup cloud
  if (cloud) {
    cloud->cleanup();
    cloud.reset();
  }
}

bool GraphicsRenderer::setupShaders() {
  try {
    lightingShader = std::make_shared<Shader>("shaders/lighting.vs.glsl",
                                              "shaders/lighting.fs.glsl");
    modelShader = std::make_shared<Shader>("shaders/model.vs.glsl",
                                           "shaders/model.fs.glsl");
    modelSimpleShader = std::make_shared<Shader>(
        "shaders/model.vs.glsl", "shaders/model-simple.fs.glsl");
    modelSimpleInstancedShader = std::make_shared<Shader>(
        "shaders/model-instanced.vs.glsl", "shaders/model-simple.fs.glsl");
    lightCubeShader = std::make_shared<Shader>("shaders/lightcube.vs.glsl",
                                               "shaders/lightcube.fs.glsl");
    windowShader = std::make_shared<Shader>("shaders/window.vs.glsl",
                                            "shaders/window.fs.glsl");
    particleShader = std::make_shared<Shader>("shaders/particle.vs.glsl",
                                              "shaders/particle.fs.glsl");
    debugShader = std::make_shared<Shader>("shaders/debug.vs.glsl",
                                           "shaders/debug.fs.glsl");
    return true;
  } catch (const std::exception &e) {
    std::cout << "Shader initialization failed: " << e.what() << std::endl;
    return false;
  }
}

bool GraphicsRenderer::loadTextures() {
  windowDiffuseMap = loadTexture(
      std::filesystem::path("resources/textures/window.png").c_str());
  return windowDiffuseMap != 0;
}

bool GraphicsRenderer::loadModels() {
  try {
    // Load table using factory
    {
      auto table = GameObject::createFromModelFile(
          "resources/objects/table/table3.obj", modelShader, "table");
      table->position = glm::vec3(0.0f, -0.2f, 2.0f);
      table->scale = glm::vec3(0.1f);
      table->interactable = false;
      // Add to GameManager
      gameManager->addObject(std::move(table));
    }

    // Load sandbox using factory
    {
      auto sandbox = GameObject::createFromModelFile(
          "resources/objects/sandbox/sandbox.obj", modelSimpleShader,
          "sandbox");
      sandbox->position = glm::vec3(0.15f, 0.13f, 2.0f);
      sandbox->scale = glm::vec3(0.13f);
      // Add to GameManager
      gameManager->addObject(std::move(sandbox));
    }

    // Load lamp using factory
    {
      auto lamp = GameObject::createFromModelFile(
          "resources/objects/lamp/lamp1.obj", modelShader, "lamp");
      lamp->position = glm::vec3{0.1f, 0.18f, 2.2f};
      lamp->scale = glm::vec3{0.03f};

      // Add lighting behaviour
      auto lampBehaviour = std::make_unique<LampLightingBehaviour>(
          lightManager, glm::vec3(0.8f, 0.8f, 0.6f), 0.2f, 0.5f);
      lamp->addBehaviour(std::move(lampBehaviour));

      // Add to GameManager
      gameManager->addObject(std::move(lamp));
    }

    // Puzzle
    setupPuzzle();

    // Trap
    setupTrap();

    return true;
  } catch (const std::exception &e) {
    std::cout << "Model loading failed: " << e.what() << std::endl;
    return false;
  }
}

bool GraphicsRenderer::setupRoomGeometry() {
  // Define vertices for a cube (used for room geometry)
  float vertices[] = {
      // positions                      // normals           // texture coords
      -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  1.0f,  0.0f, 0.0f, //
      0.5f,  -0.5f, -0.5f, 0.0f,  0.0f,  1.0f,  1.0f, 0.0f, //
      0.5f,  0.5f,  -0.5f, 0.0f,  0.0f,  1.0f,  1.0f, 1.0f, //
      0.5f,  0.5f,  -0.5f, 0.0f,  0.0f,  1.0f,  1.0f, 1.0f, //
      -0.5f, 0.5f,  -0.5f, 0.0f,  0.0f,  1.0f,  0.0f, 1.0f, //
      -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  1.0f,  0.0f, 0.0f, //

      -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  -1.0f, 0.0f, 0.0f, //
      0.5f,  -0.5f, 0.5f,  0.0f,  0.0f,  -1.0f, 0.0f, 0.0f, //
      0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  -1.0f, 1.0f, 1.0f, //
      0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  -1.0f, 1.0f, 1.0f, //
      -0.5f, 0.5f,  0.5f,  0.0f,  0.0f,  -1.0f, 0.0f, 1.0f, //
      -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  -1.0f, 0.0f, 0.0f, //

      -0.5f, 0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f, //
      -0.5f, 0.5f,  -0.5f, 1.0f,  0.0f,  0.0f,  0.0f, 0.0f, //
      -0.5f, -0.5f, -0.5f, 1.0f,  0.0f,  0.0f,  1.0f, 1.0f, //
      -0.5f, -0.5f, -0.5f, 1.0f,  0.0f,  0.0f,  1.0f, 1.0f, //
      -0.5f, -0.5f, 0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f, //
      -0.5f, 0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f, //

      0.5f,  0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,  0.0f, 0.0f, //
      0.5f,  0.5f,  -0.5f, -1.0f, 0.0f,  0.0f,  0.0f, 0.0f, //
      0.5f,  -0.5f, -0.5f, -1.0f, 0.0f,  0.0f,  1.0f, 1.0f, //
      0.5f,  -0.5f, -0.5f, -1.0f, 0.0f,  0.0f,  1.0f, 1.0f, //
      0.5f,  -0.5f, 0.5f,  -1.0f, 0.0f,  0.0f,  0.0f, 1.0f, //
      0.5f,  0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,  0.0f, 0.0f, //

      -0.5f, -0.5f, -0.5f, 0.0f,  1.0f,  0.0f,  0.0f, 0.0f, //
      0.5f,  -0.5f, -0.5f, 0.0f,  1.0f,  0.0f,  0.0f, 0.0f, //
      0.5f,  -0.5f, 0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f, //
      0.5f,  -0.5f, 0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f, //
      -0.5f, -0.5f, 0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f, //
      -0.5f, -0.5f, -0.5f, 0.0f,  1.0f,  0.0f,  0.0f, 0.0f, //

      -0.5f, 0.5f,  -0.5f, 0.0f,  -1.0f, 0.0f,  0.0f, 0.0f, //
      0.5f,  0.5f,  -0.5f, 0.0f,  -1.0f, 0.0f,  0.0f, 0.0f, //
      0.5f,  0.5f,  0.5f,  0.0f,  -1.0f, 0.0f,  1.0f, 1.0f, //
      0.5f,  0.5f,  0.5f,  0.0f,  -1.0f, 0.0f,  1.0f, 1.0f, //
      -0.5f, 0.5f,  0.5f,  0.0f,  -1.0f, 0.0f,  0.0f, 1.0f, //
      -0.5f, 0.5f,  -0.5f, 0.0f,  -1.0f, 0.0f,  0.0f, 0.0f, //
  };

  // Extract vertices for different room components
  float ceilingVertices[VERTEX_COUNT];
  float floorVertices[VERTEX_COUNT];
  float leftWallVertices[VERTEX_COUNT];
  float rightWallVertices[VERTEX_COUNT];
  float frontWallVertices[VERTEX_COUNT];

  // Copy vertices for each component (this is simplified - in a real
  // implementation you would extract the appropriate faces)
  std::memcpy(ceilingVertices, vertices + VERTEX_COUNT * 5,
              sizeof(ceilingVertices));
  std::memcpy(floorVertices, vertices + VERTEX_COUNT * 4,
              sizeof(floorVertices));
  std::memcpy(rightWallVertices, vertices + VERTEX_COUNT * 3,
              sizeof(rightWallVertices));
  std::memcpy(leftWallVertices, vertices + VERTEX_COUNT * 2,
              sizeof(leftWallVertices));
  std::memcpy(frontWallVertices, vertices, sizeof(frontWallVertices));

  // Setup geometry components
  setupGeometryComponent(ceiling, ceilingVertices, VERTEX_COUNT);
  setupGeometryComponent(floor, floorVertices, VERTEX_COUNT);
  setupGeometryComponent(leftWall, leftWallVertices, VERTEX_COUNT);
  setupGeometryComponent(rightWall, rightWallVertices, VERTEX_COUNT);
  setupGeometryComponent(frontWall, frontWallVertices, VERTEX_COUNT);
  setupGeometryComponent(lightCube, vertices,
                         VERTEX_COUNT * 6); // All vertices for the light cube

  // Generate line geometry for unit cube (for AABB debug visualization)
  // 12 lines, 24 vertices, each vertex 3 floats (x, y, z)
  float debugLineVertices[] = {
      // Bottom square
      -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f,
      0.5f, 0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f,
      -0.5f, -0.5f,
      // Top square
      -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
      0.5f, 0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f,
      -0.5f,
      // Vertical edges
      -0.5f, -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f,
      -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f,
      0.5f, 0.5f};
  // Note: debug shader expects only position attribute (location 0)
  // We'll use a custom setup for lines (different vertex format)
  // Create VAO/VBO for line geometry
  glGenVertexArrays(1, &debugCubeLines.VAO);
  glGenBuffers(1, &debugCubeLines.VBO);
  glBindVertexArray(debugCubeLines.VAO);
  glBindBuffer(GL_ARRAY_BUFFER, debugCubeLines.VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(debugLineVertices), debugLineVertices,
               GL_STATIC_DRAW);
  // Position attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glBindVertexArray(0);
  debugCubeLines.vertexCount = 24; // 24 vertices for lines

  return true;
}

void GraphicsRenderer::setupGeometryComponent(RoomGeometry &geometry,
                                              const float *vertices,
                                              size_t vertexCount) {
  glGenVertexArrays(1, &geometry.VAO);
  glGenBuffers(1, &geometry.VBO);

  glBindVertexArray(geometry.VAO);
  glBindBuffer(GL_ARRAY_BUFFER, geometry.VBO);
  glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(float), vertices,
               GL_STATIC_DRAW);

  // Position attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, VERTEX_SIZE * sizeof(float),
                        (void *)0);
  glEnableVertexAttribArray(0);

  // Normal attribute
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, VERTEX_SIZE * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  // Texture coordinate attribute
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, VERTEX_SIZE * sizeof(float),
                        (void *)(6 * sizeof(float)));
  glEnableVertexAttribArray(2);

  glBindVertexArray(0);
  geometry.vertexCount = vertexCount;
}

void GraphicsRenderer::cleanupGeometryComponent(RoomGeometry &geometry) {
  if (geometry.VAO != 0) {
    glDeleteVertexArrays(1, &geometry.VAO);
    geometry.VAO = 0;
  }
  if (geometry.VBO != 0) {
    glDeleteBuffers(1, &geometry.VBO);
    geometry.VBO = 0;
  }
  geometry.vertexCount = 0;
}

void GraphicsRenderer::renderRoom(const glm::mat4 &projection,
                                  const glm::mat4 &view,
                                  const glm::vec3 &cameraPosition) {
  lightingShader->use();

  // Set common uniforms
  lightingShader->setMat4("projection", projection);
  lightingShader->setMat4("view", view);
  // Lighting handled by UBO

  glm::mat4 model = glm::mat4(1.0f);

  // Render ceiling
  lightingShader->setVec3("material.ambient", glm::vec3(0.2f, 0.2f, 0.2f));
  lightingShader->setVec3("material.diffuse", glm::vec3(1.0f, 1.0f, 1.0f));
  lightingShader->setVec3("material.specular", glm::vec3(0.1f, 0.1f, 0.1f));
  lightingShader->setFloat("material.shininess", 64.0f);

  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(0.0f, 0.3f, 2.0f));
  model = glm::scale(model, glm::vec3(2.0f, 1.0f, 1.0f));
  lightingShader->setMat4("model", model);

  glBindVertexArray(ceiling.VAO);
  glDrawArrays(GL_TRIANGLES, 0, ceiling.vertexCount);

  // Render floor
  lightingShader->setVec3("material.ambient", glm::vec3(0.2f, 0.2f, 0.2f));
  lightingShader->setVec3("material.diffuse", glm::vec3(0.5f, 0.5f, 0.5f));
  lightingShader->setVec3("material.specular", glm::vec3(0.3f, 0.3f, 0.3f));
  lightingShader->setFloat("material.shininess", 64.0f);

  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(0.0f, 0.3f, 2.0f));
  model = glm::scale(model, glm::vec3(2.0f, 1.0f, 1.0f));
  lightingShader->setMat4("model", model);

  glBindVertexArray(floor.VAO);
  glDrawArrays(GL_TRIANGLES, 0, floor.vertexCount);

  // Render left wall
  lightingShader->setVec3("material.ambient", glm::vec3(0.2f, 0.2f, 0.2f));
  lightingShader->setVec3("material.diffuse", glm::vec3(1.0f, 0.0f, 0.31f));
  lightingShader->setVec3("material.specular", glm::vec3(1.0f, 0.0f, 0.31f));
  lightingShader->setFloat("material.shininess", 64.0f);

  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(0.0f, 0.3f, 2.0f));
  model = glm::translate(model, glm::vec3(-0.5f, 0.0f, 0.0f));
  lightingShader->setMat4("model", model);

  glBindVertexArray(leftWall.VAO);
  glDrawArrays(GL_TRIANGLES, 0, leftWall.vertexCount);

  // Render right wall
  lightingShader->setVec3("material.ambient", glm::vec3(0.2f, 0.2f, 0.2f));
  lightingShader->setVec3("material.diffuse", glm::vec3(1.0f, 0.0f, 0.31f));
  lightingShader->setVec3("material.specular", glm::vec3(1.0f, 0.0f, 0.31f));
  lightingShader->setFloat("material.shininess", 64.0f);

  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(0.0f, 0.3f, 2.0f));
  model = glm::translate(model, glm::vec3(0.5f, 0.0f, 0.0f));
  lightingShader->setMat4("model", model);

  glBindVertexArray(rightWall.VAO);
  glDrawArrays(GL_TRIANGLES, 0, rightWall.vertexCount);

  // Render front wall with window
  windowShader->use();
  windowShader->setMat4("projection", projection);
  windowShader->setMat4("view", view);

  // Lighting handled by UBO (binding = 1)

  windowShader->setVec3("material.ambient", glm::vec3(0.5f, 0.25f, 0.0f));
  windowShader->setVec3("material.diffuse", glm::vec3(0.5f, 0.25f, 0.0f));
  windowShader->setVec3("material.specular", glm::vec3(0.5f, 0.25f, 0.0f));
  windowShader->setFloat("material.shininess", 64.0f);

  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(0.0f, 0.3f, 2.0f));
  model = glm::scale(model, glm::vec3(2.0f, 1.0f, 1.0f));
  windowShader->setMat4("model", model);

  // Texture parameters for window
  windowShader->setVec2("texScale", glm::vec2(0.3f, 0.6f));
  windowShader->setVec2("texOffset", glm::vec2(0.0f, 0.2f));

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, windowDiffuseMap);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glBindVertexArray(frontWall.VAO);
  glDrawArrays(GL_TRIANGLES, 0, frontWall.vertexCount);
}

void GraphicsRenderer::renderObjects(const glm::mat4 &projection,
                                     const glm::mat4 &view) {
  for (const auto &obj : gameManager->getObjects()) {
    obj->render(projection, view);
  }
}

void GraphicsRenderer::renderLightCube(const glm::mat4 &projection,
                                       const glm::mat4 &view) {
  lightCubeShader->use();
  lightCubeShader->setMat4("projection", projection);
  lightCubeShader->setMat4("view", view);

  glm::mat4 model = glm::mat4(1.0f);
  model = glm::translate(model, lightPosition);
  model = glm::scale(model, glm::vec3(0.1f));
  lightCubeShader->setMat4("model", model);

  glBindVertexArray(lightCube.VAO);
  glDrawArrays(GL_TRIANGLES, 0, lightCube.vertexCount);
}

void GraphicsRenderer::renderTerrain(const glm::mat4 &projection,
                                     const glm::mat4 &view,
                                     const glm::vec3 &cameraPosition) {
  if (!terrainMesh)
    return;

  modelSimpleShader->use();

  // Lighting handled by UBO
  modelSimpleShader->setFloat("material.shininess", 2.0f);
  modelSimpleShader->setMat4("projection", projection);
  modelSimpleShader->setMat4("view", view);
  modelSimpleShader->setVec3("material.ambient", glm::vec3(0.8, 1.0, 0.8));
  modelSimpleShader->setVec3("material.diffuse", glm::vec3(0.8, 1.0, 0.8));
  modelSimpleShader->setVec3("material.specular", glm::vec3(0.2, 0.4, 0.2));

  modelSimpleShader->setMat4("model", terrainModel);

  terrainMesh->render(*modelSimpleShader);
}

void GraphicsRenderer::renderParticles(const glm::mat4 &projection,
                                       const glm::mat4 &view,
                                       const glm::vec3 &cameraPosition) {

  {
    static float lastTime = 0.0f;
    float currentTime = static_cast<float>(glfwGetTime());
    float deltaTime = currentTime - lastTime;
    lastTime = currentTime;

    // Update particle systems (includes automatic emission)
    snowSystem->update(deltaTime);
    rainSystem->update(deltaTime);
    if (orbAuraSystem) {
      orbAuraSystem->update(deltaTime);
    }
  }

  glEnable(GL_PROGRAM_POINT_SIZE);
  particleShader->use();
  particleShader->setVec3("viewPos", cameraPosition);

  snowSystem->render(view, projection);
  rainSystem->render(view, projection);
  if (orbAuraSystem) {
    orbAuraSystem->render(view, projection);
  }
  glDisable(GL_PROGRAM_POINT_SIZE);

  // Render test cube system
  if (testCubeSystem && testCubeSystem->isVisible()) {
    testCubeSystem->render(view, projection);
  }
}

void GraphicsRenderer::renderDebugAABBs(const glm::mat4 &projection,
                                        const glm::mat4 &view) {
  if (!debugAABBsEnabled)
    return;
  if (!debugShader)
    return;

  debugShader->use();
  debugShader->setMat4("projection", projection);
  debugShader->setMat4("view", view);

  // Iterate through all objects in GameManager
  const auto &objects = gameManager->getObjects();
  for (GameObject *obj : objects) {
    if (!obj) {
      continue;
    }

    // Set color for AABB lines (interactable green, otherwise red)
    if (obj->interactable) {
      debugShader->setVec3("color", glm::vec3(0.0f, 1.0f, 0.0f));
    } else {
      debugShader->setVec3("color", glm::vec3(1.0f, 0.0f, 0.0f));
    }

    // Get world AABB
    scene::AABB aabb = obj->getWorldAABB();
    renderDebugAABB(aabb);
  }

    // Compute scale and translation from AABB min/max
    glm::vec3 center = (aabb.min + aabb.max) * 0.5f;
    glm::vec3 size = aabb.max - aabb.min;

    // Create model matrix: translate to center, scale to size
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, center);
    // unit cube is from -0.5 to 0.5, scaling expands
    model = glm::scale(model, size);

  // Render debug AABB for test cube particles (yellow color)
  if (testCubeSystem && testCubeSystem->isVisible()) {
    // Yellow
    debugShader->setVec3("color", glm::vec3(1.0f, 1.0f, 0.0f));

    // Get individual world AABBs for particles
    const auto cubeAABBs = testCubeSystem->getParticleWorldAABBs();
    for (const auto &cubeAABB : cubeAABBs) {
      renderDebugAABB(cubeAABB);
    }
  }
}

void GraphicsRenderer::setupPuzzle() {
  // Cubes for animation testing using factory
  const auto createCube = getCreateCube(gameManager, modelSimpleShader);

  auto createPuzzleCube = [&](glm::vec3 position, glm::vec3 color,
                              float size = 0.1f, const std::string &name = "") {
    auto cubeObj = createCube(position, color, size, name);

    // Add animation behaviour: when selected, move up by 0.2f
    cubeObj->addBehaviour(std::make_unique<PuzzleMovementBehaviour>(
        gameManager->getPuzzleManager(),
        position + glm::vec3(0.0f, 0.2f, 0.0f)));
    return cubeObj;
  };

  // Puzzle
  createPuzzleCube(glm::vec3(-0.3f, 0.2f, 2.0f), glm::vec3(1.0f, 0.0f, 0.0f),
                   0.1f, "red_cube");
  createPuzzleCube(glm::vec3(-0.1f, 0.2f, 2.0f), glm::vec3(0.0f, 1.0f, 0.0f),
                   0.1f, "green_cube");
  createPuzzleCube(glm::vec3(0.1f, 0.2f, 2.0f), glm::vec3(0.0f, 0.0f, 1.0f),
                   0.1f, "blue_cube");

  auto hiddenCube =
      createPuzzleCube(glm::vec3(0.3f, 0.2f, 2.0f), glm::vec3(0.0f, 0.0f, 1.0f),
                       0.1f, "hidden_cube");

  hiddenCube->addBehaviour(std::make_unique<HiddenCellBehaviour>(
      gameManager->getPuzzleManager(), hiddenCube->position,
      glm::vec3{90.0f, 0.0f, 0.0f}));

  // Floor hidden compartment with spirit orb
  setupFloorCompartment();
}

void GraphicsRenderer::setupFloorCompartment() {
  // Position on floor near puzzle cubes
  const glm::vec3 floorPosition = glm::vec3(0.0f, -0.2f, 2.0f); // Floor level
  const glm::vec3 cavityPosition = floorPosition;
  const glm::vec3 orbPosition = cavityPosition; // Orb inside cavity

  // Cavity (hidden compartment below floor)
  GameObject *cavityObj = [this, &cavityPosition] {
    Material cavityMaterial{};
    cavityMaterial.type = MaterialType::UNIFORM;
    cavityMaterial.ambient = glm::vec3(0.05f, 0.05f, 0.05f);
    cavityMaterial.diffuse = glm::vec3(0.2f, 0.2f, 0.2f); // Dark gray
    cavityMaterial.specular = glm::vec3(0.02f);
    cavityMaterial.shininess = 8.0f;

    auto cavity =
        ModelFactory::createCube(0.1f, cavityMaterial, "floor_cavity");
    auto cavityObj = std::make_unique<GameObject>(std::move(cavity),
                                                  modelShader, "floor_cavity");
    cavityObj->position = cavityPosition;
    cavityObj->scale = glm::vec3(0.0f); // Start invisible
    cavityObj->interactable = false;

    return gameManager->addObject(std::move(cavityObj));
  }();

  // Spirit orb (placeholder cube with emissive material)
  GameObject *orbObj = [this, &orbPosition] {
    Material orbMaterial{};
    orbMaterial.type = MaterialType::UNIFORM;
    orbMaterial.ambient = glm::vec3(0.1f, 0.4f, 0.5f);
    orbMaterial.diffuse = glm::vec3(0.2f, 0.8f, 1.0f); // Cyan
    orbMaterial.specular = glm::vec3(0.5f);
    orbMaterial.shininess = 64.0f;
    orbMaterial.setEmissive(true, glm::vec3(0.2f, 0.8f, 1.0f), 1.0f);

    auto orb = ModelFactory::createSphere(0.025f, orbMaterial, "spirit_orb");
    auto orbObj = std::make_unique<GameObject>(std::move(orb),
                                               modelSimpleShader, "spirit_orb");
    orbObj->position = orbPosition;
    orbObj->scale = glm::vec3(0.0f); // Start invisible
    orbObj->interactable = false;
    orbObj->stopAnimation();

    // Add OrbGlowBehaviour for pulsating glow
    orbObj->addBehaviour(std::make_unique<OrbGlowBehaviour>());

    // After revealed, make it interactable
    gameManager->getPuzzleManager()->registerPuzzleCallback(
        orbObj.get(), [orbObj = orbObj.get()] { orbObj->interactable = true; });

    return gameManager->addObject(std::move(orbObj));
  }();

  // Floor tile (hidden compartment door)
  {
    Material floorTileMaterial;
    floorTileMaterial.type = MaterialType::UNIFORM;
    floorTileMaterial.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
    floorTileMaterial.diffuse = glm::vec3(0.5f, 0.5f, 0.5f);
    floorTileMaterial.specular = glm::vec3(0.1f);
    floorTileMaterial.shininess = 32.0f;

    auto floorTile =
        ModelFactory::createCube(0.1f, floorTileMaterial, "floor_tile");
    auto tileObj = std::make_unique<GameObject>(std::move(floorTile),
                                                modelShader, "floor_tile");
    tileObj->position = floorPosition;
    // Thin tile, flush with floor
    tileObj->scale = glm::vec3(0.2f, 0.01f, 0.2f);
    // Initially locked until puzzle solved
    tileObj->interactable = false;

    // Add behaviour that unlocks when puzzle is solved
    auto tileBehaviour = std::make_unique<FloorCompartmentBehaviour>(
        gameManager->getPuzzleManager(),
        cavityObj, // Cavity GameObject pointer
        orbObj,    // Orb GameObject pointer
        [this, orbObj]() {
          std::println(std::clog, "Orb glow triggered!");
          activateOrbAura(orbObj);
        },
        glm::vec3(0.2f, 0.1f, 0.2f), // Cavity size
        1.2f                         // Reveal duration
    );
    tileObj->addBehaviour(std::move(tileBehaviour));

    // Add to GameManager
    gameManager->addObject(std::move(tileObj));
  }
}

void GraphicsRenderer::setupTrap() {
  const auto createCube = getCreateCube(gameManager, modelSimpleShader);

  { // Bookcase
    auto bookcase = GameObject::createFromModelFile(
        "resources/objects/bookcase/bookcase1.obj", modelShader, "book_case");
    bookcase->position = glm::vec3(0.92f, 0.15f, 2.0f);
    bookcase->scale = glm::vec3(0.04f);
    bookcase->rotation = glm::vec3(0.0f, -90.0f, 0.0f);
    bookcase->interactable = false;

    // Add lighting behaviour
    auto bookCaseBehaviour = std::make_unique<TrapBehaviour>(
        gameManager->getTrapManager(),
        bookcase->position + glm::vec3{0.0f, 0.0f, 0.6f}, bookcase->rotation);
    bookcase->addBehaviour(std::move(bookCaseBehaviour));

    // Add to GameManager
    gameManager->addObject(std::move(bookcase));
  }

  { // Trigger that moves the bookcase
    auto triggerCube =
        createCube(glm::vec3{0.1f, 0.13f, 1.8f}, glm::vec3{1.0f, 1.0f, 1.0f},
                   0.05f, "trigger_cube");

    triggerCube->addBehaviour(
        std::make_unique<TrapTriggerBehaviour>(gameManager->getTrapManager()));
  }

  // Position on right wall (behind bookcase)
  const glm::vec3 panelPosition = glm::vec3(0.98f, 0.15f, 2.0f);
  const glm::vec3 cavityPosition = panelPosition + glm::vec3(0.01f, 0.2f, 0.0f);

  // Cavity
  GameObject *cavityObj = [this, &cavityPosition] {
    Material cavityMaterial{};
    cavityMaterial.type = MaterialType::UNIFORM;
    cavityMaterial.ambient = glm::vec3(0.1f, 0.1f, 0.1f);
    cavityMaterial.diffuse = glm::vec3(0.3f, 0.3f, 0.3f); // Gray
    cavityMaterial.specular = glm::vec3(0.05f);
    cavityMaterial.shininess = 16.0f;

    auto cavity = ModelFactory::createCube(0.1f, cavityMaterial, "wall_cavity");
    auto cavityObj = std::make_unique<GameObject>(std::move(cavity),
                                                  modelShader, "wall_cavity");
    cavityObj->position = cavityPosition;
    cavityObj->rotation = glm::vec3(0.0f, -90.0f, 0.0f);
    cavityObj->scale = glm::vec3(0.0f); // Start invisible
    cavityObj->interactable = false;

    // Return pointer for behaviour
    return gameManager->addObject(std::move(cavityObj));
  }();

  { // Wall panel (hidden compartment door) on opposite wall
    Material wallPanelMaterial;
    wallPanelMaterial.type = MaterialType::UNIFORM;
    wallPanelMaterial.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
    wallPanelMaterial.diffuse = glm::vec3(0.6f, 0.3f, 0.1f); // Brown wood-like
    wallPanelMaterial.specular = glm::vec3(0.1f);
    wallPanelMaterial.shininess = 32.0f;

    // Create cavity (hidden compartment inside wall)
    auto wallPanel =
        ModelFactory::createCube(0.1f, wallPanelMaterial, "wall_panel");
    auto panelObj = std::make_unique<GameObject>(std::move(wallPanel),
                                                 modelShader, "wall_panel");
    panelObj->position = panelPosition;
    panelObj->rotation = glm::vec3(0.0f, -90.0f, 0.0f);
    panelObj->scale = glm::vec3(0.2f, 0.2f, 0.001f); // Thin panel
    panelObj->interactable = false;                  // Initially locked

    // Add behaviour that unlocks when bookcase moves
    auto panelBehaviour = std::make_unique<WallPanelBehaviour>(
        gameManager->getTrapManager(),
        cavityObj, // Cavity GameObject pointer
        []() { std::println(std::clog, "Arrow triggerred!"); },
        glm::vec3(0.3f, 0.3f, 0.1f), // Cavity size
        0.8f                         // Reveal duration
    );
    panelObj->addBehaviour(std::move(panelBehaviour));

    // Add to GameManager
    gameManager->addObject(std::move(panelObj));
  }
}

void GraphicsRenderer::renderDebugAABB(const scene::AABB &aabb) const {
  const auto model = [&aabb] {
    const glm::vec3 center = (aabb.min + aabb.max) * 0.5f;
    const glm::vec3 size = aabb.max - aabb.min;

    auto model = glm::mat4{1.0f};
    // translate to center
    model = glm::translate(model, center);
    // scale to size
    model = glm::scale(model, size);
    return model;
  }();

  debugShader->setMat4("model", model);

  // Draw lines
  glBindVertexArray(debugCubeLines.VAO);
  glDrawArrays(GL_LINES, 0, debugCubeLines.vertexCount);
  glBindVertexArray(0);
}
