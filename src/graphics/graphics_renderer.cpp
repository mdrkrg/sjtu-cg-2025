#include "core/application.h"
#include "graphics/graphics_renderer.h"
#include <GLFW/glfw3.h>
#include "graphics/particles/particle_factory.hpp"
#include "graphics/particles/arrow_launcher.hpp"
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

using graphics::particles::ParticleFactory;

const glm::vec3 GraphicsRenderer::weatherPosition = {0.0f, 0.0f, 0.0f};

// Scale terrain to fit on the table
const glm::mat4 GraphicsRenderer::terrainModel = glm::scale(
    // Position terrain on the table
    glm::translate(glm::mat4(1.0f), glm::vec3(0.15f, 0.15f, 2.0f)),
    glm::vec3(0.24f));

// Position on right wall (behind bookcase)
static constexpr glm::vec3 panelPosition = glm::vec3(1.22f, 0.15f, 2.0f);
static constexpr glm::vec3 cavityPosition =
    panelPosition + glm::vec3(0.01f, 0.2f, 0.0f);
static constexpr glm::vec3 cavityArrowPosition =
    cavityPosition + glm::vec3(0.2f, 0.0f, 0.0f);

GraphicsRenderer::GraphicsRenderer(std::shared_ptr<GameManager> gameManager)
    : lightingShader(nullptr), modelShader(nullptr), modelSimpleShader(nullptr),
      modelSimpleInstancedShader(nullptr), modelInstancedShader(nullptr),
      lightCubeShader(nullptr), windowShader(nullptr), particleShader(nullptr),
      debugShader(nullptr), windowDiffuseMap(0), gameManager(gameManager),
      selectedObject(nullptr),
      terrainMesh(std::make_shared<TerrainMesh>(1024, 1024, 0.2f)),
      orbAuraSystem(nullptr), arrowLauncher(nullptr), cloud(nullptr) {
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

  // Initialize post-processing system
  const auto [width, height] = Application::getInstance()->windowSize();
  postProcessingManager =
      std::make_unique<graphics::postprocessing::PostProcessingManager>();
  if (!postProcessingManager->initialize(width, height)) {
    std::cerr << "Failed to initialize post-processing system";
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

  if (!setupGeometry()) {
    std::cout << "Failed to setup room geometry" << std::endl;
    return false;
  }

  if (!setupRoomGameObjects()) {
    std::cout << "Failed to setup room GameObjects" << std::endl;
    return false;
  }

  // Initialize cloud
  {
    cloud = std::make_shared<Cloud>();
    if (not cloud->initialize()) {
      std::println(std::cerr, "Failed to initialize volumetric cloud");
      return false;
    }

    // Position the cloud above the table
    cloud->setPosition(glm::vec3(0.0f, 0.65f, 2.0f));
    cloud->setScale(glm::vec3(2.0f, 0.5f, 2.0f));

    rainSystem = ParticleFactory::createRainSystem(
        terrainMesh, terrainModel, particleShader, weatherPosition, 100000,
        500.f, cloud->getGameObject());
    snowSystem = ParticleFactory::createSnowSystem(
        terrainMesh, terrainModel, particleShader, weatherPosition, 1000000,
        500.f, cloud->getGameObject());
    cloud->initializeWeather(rainSystem, snowSystem);
  }

  // Initialize arrow launcher with multiple emitters
  arrowLauncher = std::make_shared<graphics::particles::ArrowLauncher>(
      gameManager, graphics::particles::EmissionPolicy::RoundRobin);

  auto arrowModelWithMaterials =
      ModelFactory::loadModel("resources/objects/arrow/arrow1.obj");

  arrowLauncher->init(std::move(arrowModelWithMaterials), modelInstancedShader,
                      100);

  arrowLauncher->addEmitter(glm::vec3{0.0f, 1.5f, 0.0f},  // Center emitter
                            glm::vec3{0.0f, -0.5f, 1.0f}, // Direction
                            5.0f,                         // Speed
                            10.0f                         // Spread angle
  );
  arrowLauncher->addEmitter(cavityArrowPosition,          // Right emitter
                            glm::vec3{-1.0f, 0.0f, 0.0f}, // Direction
                            5.0f,                         // Speed
                            10.0f                         // Spread angle
  );

  // Add collision targets
  for (auto obj : gameManager->getObjects()) {
    static const std::unordered_set<std::string> excluded{
        "wall_cavity",
        "plant",
        "lion",
        "carpet",
    };
    if (excluded.contains(obj->getName())) {
      continue;
    }
    arrowLauncher->addCollisionTarget(obj);
  }

  return true;
}

void GraphicsRenderer::preRender(const glm::mat4 &projection,
                                 const glm::mat4 &view,
                                 const glm::vec3 &cameraPosition) {
  (void)projection;
  (void)view;
  // Update LightManager with camera position
  lightManager.setViewPos(cameraPosition);
  // Send data to to GPU
  lightManager.updateUBO();
}

void GraphicsRenderer::render(const glm::mat4 &projection,
                              const glm::mat4 &view,
                              const glm::vec3 &cameraPosition) {
  preRender(projection, view, cameraPosition);

  if (postProcessingManager and postProcessingManager->isValid()) {
    postProcessingManager->beginRender();
  }

  // Render scene
  renderDirect(projection, view, cameraPosition);

  if (postProcessingManager and postProcessingManager->isValid()) {
    postProcessingManager->endRender(projection, view);
  }
}

void GraphicsRenderer::renderDirect(const glm::mat4 &projection,
                                    const glm::mat4 &view,
                                    const glm::vec3 &cameraPosition) {
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
  if (paused) {
    return;
  }
  // Update game objects (animations) via GameManager
  gameManager->update(deltaTime);

  // Update cloud
  if (cloud && cloud->isInitialized()) {
    cloud->update(deltaTime);
  }

  // Update arrow launcher
  if (arrowLauncher && arrowLauncher->isInitialized()) {
    arrowLauncher->update(deltaTime);
  }
}

void GraphicsRenderer::handleMouseClick(const math::Ray &ray) {
  // Use GameManager for ray casting and selection
  GameObject *selected = gameManager->handleRayCast(ray);
  if (selected) {
    std::cout << "Selected object: " << selected->getName() << std::endl;
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

  // Cleanup post-processing system
  postProcessingManager.reset();

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
    modelInstancedShader = std::make_shared<Shader>(
        "shaders/model-instanced.vs.glsl", "shaders/model.fs.glsl");
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
  static const std::filesystem::path texturePath{"resources/textures"};
  windowDiffuseMap = loadTexture(texturePath / "window.png");
  std::println(std::clog, "Loaded window texture: {}", windowDiffuseMap);

  // Load floor texture
  floorTexture.path = texturePath / "dark_wooden_planks_4k.blend" / "textures" /
                      "dark_wooden_planks_diff_4k.jpg";
  floorTexture.type = "texture_diffuse";
  floorTexture.id = loadTexture(floorTexture.path);
  std::println(std::clog, "Loaded floor texture: {}", floorTexture.id);

  // Load wall texture
  wallTexture.path = texturePath / "decrepit_wallpaper_1k.blend" / "textures" /
                     "decrepit_wallpaper_diff_1k.jpg";
  wallTexture.type = "texture_diffuse";
  wallTexture.id = loadTexture(wallTexture.path);

  std::println(std::clog, "Loaded wall texture: {}", wallTexture.id);

  return windowDiffuseMap != 0 && floorTexture.id != 0 && wallTexture.id != 0;
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
      lamp->position = glm::vec3{0.08f, 0.18f, 2.2f};
      lamp->scale = glm::vec3{0.03f};

      // Add lighting behaviour with callback
      auto lampBehaviour = std::make_unique<LampLightingBehaviour>(
          lightManager, glm::vec3(0.8f, 0.8f, 0.6f), 0.2f, 0.5f,
          [this](GameObject *lampObj, bool on) {
            if (not postProcessingManager) {
              return;
            }
            if (on) {
              postProcessingManager->enableLampAura(lampObj, 0.8f);
            } else {
              postProcessingManager->disableLampAura();
            }
          });
      lamp->addBehaviour(std::move(lampBehaviour));

      // Add to GameManager
      gameManager->addObject(std::move(lamp));
    }

    { // window
      auto window = GameObject::createFromModelFile(
          "resources/objects/window/window.fbx", modelSimpleShader, "window");
      window->position = glm::vec3(0.0f, 0.35f, 0.75f + 0.5f - 0.05f);
      window->scale = glm::vec3(0.0015f);
      // Add to GameManager
      gameManager->addObject(std::move(window));
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

bool GraphicsRenderer::setupGeometry() {
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
      0.5f,  -0.5f, -0.5f, 0.0f,  1.0f,  0.0f,  1.0f, 0.0f, //
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

  // Setup geometry components
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

bool GraphicsRenderer::setupRoomGameObjects() {
  // Define materials for room components
  Material wallMaterial(32.0f);
  wallMaterial.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
  wallMaterial.diffuse = glm::vec3(0.5f, 0.5f, 0.5f);
  wallMaterial.specular = glm::vec3(0.1f, 0.1f, 0.1f);
  wallMaterial.hasNormalMap = false;

  Material floorMaterial(64.0f);
  floorMaterial.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
  floorMaterial.diffuse = glm::vec3(0.5f, 0.5f, 0.5f);
  floorMaterial.specular = glm::vec3(0.3f, 0.3f, 0.3f);
  floorMaterial.hasNormalMap = false;

  // Room center is at (0.0f, 0.3f, 2.0f)
  // Wall thickness: 0.1f

  {
    auto ceilingModel = ModelFactory::createWall(glm::vec3(3.0f, 0.1f, 1.5f),
                                                 wallMaterial, "ceiling");
    auto ceilingObj = std::make_unique<GameObject>(std::move(ceilingModel),
                                                   modelShader, "ceiling");
    ceilingObj->position =
        glm::vec3(0.0f, 0.5f + 0.5f - 0.05f, 2.0f); // Top of room, centered
    ceilingObj->scale = glm::vec3(1.0f);
    ceilingObj->interactable = false;

    if (ceilingObj->getModel() and floorTexture.id != 0) {
      ceilingObj->getModel()->loadTexture(floorTexture);
    }

    gameManager->addObject(std::move(ceilingObj));
  }

  {
    auto floorModel = ModelFactory::createWall(glm::vec3(3.0f, 0.1f, 1.5f),
                                               floorMaterial, "floor");
    auto floorObj = std::make_unique<GameObject>(std::move(floorModel),
                                                 modelShader, "floor");
    floorObj->position =
        glm::vec3(0.0f, 0.2f - 0.5f + 0.05f, 2.0f); // Bottom of room, centered
    floorObj->scale = glm::vec3(1.0f);
    floorObj->interactable = false;

    if (floorObj->getModel() && floorTexture.id != 0) {
      floorObj->getModel()->loadTexture(floorTexture);
    }

    gameManager->addObject(std::move(floorObj));
  }

  {
    auto leftWallModel = ModelFactory::createWall(glm::vec3(0.1f, 1.2f, 1.5f),
                                                  wallMaterial, "left_wall");
    auto leftWallObj = std::make_unique<GameObject>(std::move(leftWallModel),
                                                    modelShader, "left_wall");
    leftWallObj->position =
        glm::vec3(-1.5f + 0.05f, 0.4f, 2.0f); // Left side, centered
    leftWallObj->scale = glm::vec3(1.0f);
    leftWallObj->interactable = false;

    if (leftWallObj->getModel() and wallTexture.id != 0) {
      leftWallObj->getModel()->loadTexture(wallTexture);
    }

    gameManager->addObject(std::move(leftWallObj));
  }

  {
    auto rightWallModel = ModelFactory::createWall(glm::vec3(0.1f, 1.2f, 1.5f),
                                                   wallMaterial, "right_wall");
    auto rightWallObj = std::make_unique<GameObject>(std::move(rightWallModel),
                                                     modelShader, "right_wall");
    rightWallObj->position =
        glm::vec3(1.5f - 0.05f, 0.4f, 2.0f); // Right side, centered
    rightWallObj->scale = glm::vec3(1.0f);
    rightWallObj->interactable = false;

    if (rightWallObj->getModel() and wallTexture.id != 0) {
      rightWallObj->getModel()->loadTexture(wallTexture);
    }

    gameManager->addObject(std::move(rightWallObj));
  }

  {
    // Create four separate wall pieces to form a frame around the window
    // Total wall: 3.0m wide, 1.3m high, 0.1m thick
    // Window: 0.6m wide, 0.6m high, centered
    const float windowWidth = 0.6f;
    const float windowHeight = 0.6f;
    const float wallZ = 0.75f + 0.5f - 0.05f;

    { // Top piece
      float topWallHeight = (1.3f - windowHeight) / 2.0f;
      auto topWallModel = ModelFactory::createWall(
          // Full width, height above window
          glm::vec3(3.0f, topWallHeight, 0.1f), wallMaterial, "front_wall_top");
      auto topWallObj = std::make_unique<GameObject>(
          std::move(topWallModel), modelShader, "front_wall_top");
      // Position: centered horizontally, above window
      topWallObj->position = glm::vec3(0.0f, // Centered horizontally
                                       0.35f + windowHeight / 2.0f +
                                           topWallHeight / 2.0f, // Above window
                                       wallZ);
      topWallObj->scale = glm::vec3(1.0f);
      topWallObj->interactable = false;

      if (topWallObj->getModel() and wallTexture.id != 0) {
        topWallObj->getModel()->loadTexture(wallTexture);
      }

      gameManager->addObject(std::move(topWallObj));
    }

    { // Bottom piece
      float bottomWallHeight = (1.3f - windowHeight) / 2.0f;
      auto bottomWallModel = ModelFactory::createWall(
          // Full width, height below window
          glm::vec3(3.0f, bottomWallHeight, 0.1f), wallMaterial,
          "front_wall_bottom");
      auto bottomWallObj = std::make_unique<GameObject>(
          std::move(bottomWallModel), modelShader, "front_wall_bottom");
      // Position: centered horizontally, below window
      bottomWallObj->position = glm::vec3(
          0.0f, // Centered horizontally
          0.35f - windowHeight / 2.0f - bottomWallHeight / 2.0f, // Below window
          wallZ);
      bottomWallObj->scale = glm::vec3(1.0f);
      bottomWallObj->interactable = false;

      if (bottomWallObj->getModel() and wallTexture.id != 0) {
        bottomWallObj->getModel()->loadTexture(wallTexture);
      }

      gameManager->addObject(std::move(bottomWallObj));
    }

    { // Left piece
      float leftWallWidth = (3.0f - windowWidth) / 2.0f;
      auto leftWallModel = ModelFactory::createWall(
          // Width left of window, window height
          glm::vec3(leftWallWidth, windowWidth, 0.1f), wallMaterial,
          "front_wall_left");
      auto leftWallObj = std::make_unique<GameObject>(
          std::move(leftWallModel), modelShader, "front_wall_left");
      // Position: left of window, centered vertically with window
      leftWallObj->position = glm::vec3(
          -windowWidth / 2.0f - leftWallWidth / 2.0f, // Left of window
          0.35f, // Centered vertically with window
          wallZ);
      leftWallObj->scale = glm::vec3(1.0f);
      leftWallObj->interactable = false;

      if (leftWallObj->getModel() and wallTexture.id != 0) {
        leftWallObj->getModel()->loadTexture(wallTexture);
      }

      gameManager->addObject(std::move(leftWallObj));
    }

    // Right piece
    {
      float rightWallWidth = (3.0f - windowWidth) / 2.0f;
      auto rightWallModel = ModelFactory::createWall(
          glm::vec3(rightWallWidth, windowWidth,
                    0.1f), // Width right of window, window height
          wallMaterial, "front_wall_right");
      auto rightWallObj = std::make_unique<GameObject>(
          std::move(rightWallModel), modelShader, "front_wall_right");
      // Position: right of window, centered vertically with window
      rightWallObj->position = glm::vec3(
          windowWidth / 2.0f + rightWallWidth / 2.0f, // Right of window
          0.35f, // Centered vertically with window
          wallZ);
      rightWallObj->scale = glm::vec3(1.0f);
      rightWallObj->interactable = false;

      if (rightWallObj->getModel() and wallTexture.id != 0) {
        rightWallObj->getModel()->loadTexture(wallTexture);
      }

      gameManager->addObject(std::move(rightWallObj));
    }
  }

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
  modelSimpleShader->setBool("material.use_texture", false);
  modelSimpleShader->setBool("material.emissive", false);
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

  // Render arrow launcher system
  if (arrowLauncher && arrowLauncher->isInitialized()) {
    arrowLauncher->render(view, projection);
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

  // Render debug AABB for arrow particles (blue color)
  if (arrowLauncher && arrowLauncher->isInitialized()) {
    auto emitterGroup = arrowLauncher->getEmitterGroup();
    if (emitterGroup) {
      // Set color for arrow particle AABB (blue)
      debugShader->setVec3("color", glm::vec3(0.0f, 0.0f, 1.0f));

      // Get AABBs from all systems in the emitter group
      const auto &systems = emitterGroup->getSystems();
      for (const auto &arrowSystem : systems) {
        if (not arrowSystem or not arrowSystem->isVisible()) {
          continue;
        }
        // Get individual world AABBs for all arrow particles in this system
        auto arrowParticleAABBs = arrowSystem->getParticleWorldAABBs();
        for (const auto &arrowAABB : arrowParticleAABBs) {
          renderDebugAABB(arrowAABB);
        }
      }
    }
  }
}

void GraphicsRenderer::setupPuzzle() {
  // Cubes for animation testing using factory
  const auto createCube = getCreateCube(gameManager, modelSimpleShader);

  // Puzzle
  {
    auto plant = GameObject::createFromModelFile(
        "resources/objects/plant/scene.gltf", modelShader, "plant");
    plant->position = glm::vec3(-0.8f, -0.2f, 2.0f);
    plant->rotation = glm::vec3(-90.0f, 0.0f, 90.0f);
    plant->scale = glm::vec3(0.2f);
    plant->addBehaviour(std::make_unique<PuzzleMovementBehaviour>(
        gameManager->getPuzzleManager(),
        plant->position + glm::vec3(0.0f, 0.0f, 0.3f), plant->rotation));
    gameManager->addObject(std::move(plant));
  }

  {
    auto lion = GameObject::createFromModelFile(
        "resources/objects/lion/scene.gltf", modelShader, "lion");
    lion->position = glm::vec3(0.24f, 0.11f, 2.25f);
    lion->rotation = glm::vec3(-90.0f, 0.0f, -90.0f);
    lion->scale = glm::vec3(0.03f);
    lion->addBehaviour(std::make_unique<PuzzleMovementBehaviour>(
        gameManager->getPuzzleManager(), lion->position,
        lion->rotation + glm::vec3(0.0f, 0.0f, 90.0f)));
    gameManager->addObject(std::move(lion));
  }

  {
    auto chineseDing = GameObject::createFromModelFile(
        "resources/objects/ding/ToExport.fbx", modelShader, "ding");
    chineseDing->position = glm::vec3(0.6f, -0.1f, 2.3f);
    chineseDing->rotation = glm::vec3(-90.0f, 0.0f, 0.0f);
    chineseDing->scale = glm::vec3(0.08f);
    chineseDing->addBehaviour(std::make_unique<PuzzleMovementBehaviour>(
        gameManager->getPuzzleManager(),
        chineseDing->position + glm::vec3(-0.5f, 0.0f, 0.0f),
        chineseDing->rotation + glm::vec3(0.0f, 0.0f, 90.0f)));
    gameManager->addObject(std::move(chineseDing));
  }

  {
    auto carpet = GameObject::createFromModelFile(
        "resources/objects/carpet/carpet1.fbx", modelShader, "carpet");
    carpet->position = glm::vec3(0.15f, -0.2f, 2.0f);
    carpet->rotation = glm::vec3(-90.0f, 0.0f, 90.0f);
    carpet->interactable = false;
    carpet->scale = glm::vec3(0.01f);
    carpet->addBehaviour(std::make_unique<HiddenCellBehaviour>(
        gameManager->getPuzzleManager(),
        carpet->position + glm::vec3{0.0f, 0.0f, -0.3f}, carpet->rotation));
    gameManager->addObject(std::move(carpet));
  }

  // Floor hidden compartment with spirit orb
  setupFloorCompartment();
}

void GraphicsRenderer::setupFloorCompartment() {
  // Position on floor near puzzle cubes
  const glm::vec3 floorPosition = glm::vec3(0.2f, -0.2f, 2.0f);
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
          if (postProcessingManager) {
            postProcessingManager->enableOrbGlow(orbObj, 1.0f);
          }
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
  { // Bookcase
    auto bookcase = GameObject::createFromModelFile(
        "resources/objects/bookcase/bookcase1.obj", modelShader, "book_case");
    bookcase->position = glm::vec3(1.2f, 0.15f, 2.0f);
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
    auto cup = GameObject::createFromModelFile(
        "resources/objects/cup/VintageSteelCup.fbx", modelShader,
        "trigger_cup");
    cup->position = glm::vec3{0.14f, 0.11f, 1.8f};
    cup->rotation = glm::vec3{90.0f, 180.0f, 0.0f};
    cup->scale = glm::vec3{0.4};
    cup->addBehaviour(
        std::make_unique<TrapTriggerBehaviour>(gameManager->getTrapManager()));
    gameManager->addObject(std::move(cup));
  }

  // Cavity
  GameObject *cavityObj = [this] {
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
        [this]() {
          std::println(std::clog, "Arrow triggerred!");
          // Fire arrows for testing (round-robin)
          arrowLauncher->launch(30);
          arrowLauncher->startContinuousFire(10);
        },
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
