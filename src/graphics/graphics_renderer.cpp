#include "graphics_renderer.h"
#include <GLFW/glfw3.h>
#include "particles/particle_factory.hpp"
#include "texture.hpp"
#include <filesystem>
#include <iostream>
#include <cstring>

static glm::mat4 weatherModel = glm::scale(glm::mat4(1.0), glm::vec3(0.04));

static glm::vec3 weatherPosition = {0.0f, 18.0f, 50.0f};

static glm::mat4 terrainModel = glm::translate( // Position terrain on the table
    glm::scale( // Scale terrain to fit on the table
        glm::mat4(1.0f), glm::vec3(0.5f)),
    glm::vec3(0.0f, 0.2f, 4.0f));

GraphicsRenderer::GraphicsRenderer()
    : lightingShader(nullptr), modelShader(nullptr), lightCubeShader(nullptr),
      windowShader(nullptr), particleShader(nullptr), windowDiffuseMap(0),
      tableModel(nullptr),
      terrainMesh(std::make_shared<TerrainMesh>(
          "resources/textures/iceland_heightmap.png", 1.f)),
      rainSystem{ParticleFactory::createRainSystem(terrainMesh, terrainModel,
                                                   weatherPosition, 100000)},
      snowSystem{ParticleFactory::createSnowSystem(terrainMesh, terrainModel,
                                                   weatherPosition, 1000000)},
      cloud(nullptr) {
  // Initialize geometry components
  ceiling = {0, 0, 0};
  floor = {0, 0, 0};
  leftWall = {0, 0, 0};
  rightWall = {0, 0, 0};
  frontWall = {0, 0, 0};
  lightCube = {0, 0, 0};
}

GraphicsRenderer::~GraphicsRenderer() { cleanup(); }

bool GraphicsRenderer::initialize() {
  if (!setupShaders()) {
    std::cout << "Failed to setup shaders" << std::endl;
    return false;
  }

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
  cloud->setPosition(glm::vec3(0.0f, 0.75f, 2.0f));
  cloud->setScale(glm::vec3(2.0f, 0.5f, 2.0f));

  return true;
}

void GraphicsRenderer::render(const glm::mat4 &projection,
                              const glm::mat4 &view,
                              const glm::vec3 &cameraPosition,
                              const glm::vec3 &lightPosition) {
  renderRoom(projection, view, cameraPosition, lightPosition);
  renderTable(projection, view, cameraPosition, lightPosition);
  renderLightCube(projection, view, lightPosition);
  // Render cloud
  if (cloud && cloud->isInitialized()) {
    cloud->render(projection, view, lightPosition, cameraPosition);
  }
  renderTerrain(projection, view, cameraPosition, lightPosition);
  renderParticles(projection, view, cameraPosition, lightPosition);
}

void GraphicsRenderer::update(float deltaTime) {
  if (cloud && cloud->isInitialized()) {
    cloud->update(deltaTime);
  }
}

void GraphicsRenderer::cleanup() {
  // Cleanup geometry
  cleanupGeometryComponent(ceiling);
  cleanupGeometryComponent(floor);
  cleanupGeometryComponent(leftWall);
  cleanupGeometryComponent(rightWall);
  cleanupGeometryComponent(frontWall);
  cleanupGeometryComponent(lightCube);

  // Cleanup shaders
  lightingShader.reset();
  modelShader.reset();
  lightCubeShader.reset();
  windowShader.reset();
  particleShader.reset();

  // Cleanup models
  tableModel.reset();
  terrainMesh.reset();

  // Cleanup cloud
  if (cloud) {
    cloud->cleanup();
    cloud.reset();
  }
}

bool GraphicsRenderer::setupShaders() {
  try {
    lightingShader = std::make_unique<Shader>("shaders/lighting.vs.glsl",
                                              "shaders/lighting.fs.glsl");
    modelShader = std::make_unique<Shader>("shaders/model.vs.glsl",
                                           "shaders/model.fs.glsl");
    lightCubeShader = std::make_unique<Shader>("shaders/lightcube.vs.glsl",
                                               "shaders/lightcube.fs.glsl");
    windowShader = std::make_unique<Shader>("shaders/window.vs.glsl",
                                            "shaders/window.fs.glsl");
    particleShader = std::make_unique<Shader>("shaders/particle.vs.glsl",
                                              "shaders/particle.fs.glsl");
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
    tableModel = std::make_unique<Model>(
        std::filesystem::path("resources/objects/table/table3.obj"));
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
                                  const glm::vec3 &cameraPosition,
                                  const glm::vec3 &lightPosition) {
  lightingShader->use();

  // Set common uniforms
  lightingShader->setMat4("projection", projection);
  lightingShader->setMat4("view", view);
  lightingShader->setVec3("viewPos", cameraPosition);
  lightingShader->setVec3("light.position", lightPosition);

  glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
  glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f);
  glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f);

  lightingShader->setVec3("light.ambient", ambientColor);
  lightingShader->setVec3("light.diffuse", diffuseColor);
  lightingShader->setVec3("light.specular", glm::vec3(1.0f));

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
  windowShader->setVec3("viewPos", cameraPosition);
  windowShader->setVec3("lightPos", lightPosition);

  windowShader->setVec3("light.ambient", ambientColor);
  windowShader->setVec3("light.diffuse", diffuseColor);
  windowShader->setVec3("light.specular", glm::vec3(1.0f));

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

void GraphicsRenderer::renderTable(const glm::mat4 &projection,
                                   const glm::mat4 &view,
                                   const glm::vec3 &cameraPosition,
                                   const glm::vec3 &lightPosition) {
  modelShader->use();

  glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
  glm::vec3 diffuseColor = lightColor * glm::vec3(0.8f);
  glm::vec3 ambientColor = lightColor * glm::vec3(0.2f);

  modelShader->setVec3("light.position", lightPosition);
  modelShader->setVec3("viewPos", cameraPosition);
  modelShader->setVec3("light.ambient", ambientColor);
  modelShader->setVec3("light.diffuse", diffuseColor);
  modelShader->setVec3("light.specular", glm::vec3(1.0f, 1.0f, 1.0f));
  modelShader->setFloat("light.constant", 1.0f);
  modelShader->setFloat("light.linear", 0.09f);
  modelShader->setFloat("light.quadratic", 0.032f);
  modelShader->setFloat("material.shininess", 32.0f);
  modelShader->setMat4("projection", projection);
  modelShader->setMat4("view", view);

  glm::mat4 model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(0.0f, -0.2f, 2.0f));
  model = glm::scale(model, glm::vec3(0.1f));
  modelShader->setMat4("model", model);

  if (tableModel) {
    tableModel->Draw(*modelShader);
  }
}

void GraphicsRenderer::renderLightCube(const glm::mat4 &projection,
                                       const glm::mat4 &view,
                                       const glm::vec3 &lightPosition) {
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
                                     const glm::vec3 &cameraPosition,
                                     const glm::vec3 &lightPosition) {
  if (!terrainMesh)
    return;

  modelShader->use();

  glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
  glm::vec3 diffuseColor = lightColor * glm::vec3(0.8f);
  glm::vec3 ambientColor = lightColor * glm::vec3(0.2f);

  modelShader->setVec3("light.position", lightPosition);
  modelShader->setVec3("viewPos", cameraPosition);
  modelShader->setVec3("light.ambient", ambientColor);
  modelShader->setVec3("light.diffuse", diffuseColor);
  modelShader->setVec3("light.specular", glm::vec3(1.0f, 1.0f, 1.0f));
  modelShader->setFloat("light.constant", 1.0f);
  modelShader->setFloat("light.linear", 0.09f);
  modelShader->setFloat("light.quadratic", 0.032f);
  modelShader->setFloat("material.shininess", 32.0f);
  modelShader->setMat4("projection", projection);
  modelShader->setMat4("view", view);

  modelShader->setMat4("model", terrainModel);

  terrainMesh->Draw(*modelShader);
}

void GraphicsRenderer::renderParticles(const glm::mat4 &projection,
                                       const glm::mat4 &view,
                                       const glm::vec3 &cameraPosition,
                                       const glm::vec3 &lightPosition) {

  {
    static float lastTime = 0.0f;
    float currentTime = static_cast<float>(glfwGetTime());
    float deltaTime = currentTime - lastTime;
    lastTime = currentTime;

    // Update particle systems (includes automatic emission)
    snowSystem->update(deltaTime, weatherModel);
    rainSystem->update(deltaTime, weatherModel);
  }

  glEnable(GL_PROGRAM_POINT_SIZE);
  particleShader->use();
  particleShader->setVec3("viewPos", cameraPosition);

  snowSystem->render(*particleShader, weatherModel, view, projection);
  rainSystem->render(*particleShader, weatherModel, view, projection);
  glDisable(GL_PROGRAM_POINT_SIZE);
}
