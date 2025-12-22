
#include "graphics/material.hpp"
#include "graphics/model_factory.hpp"
#include "scene/game_manager.hpp"
#include "scene/game_object.hpp"
#include <memory>

inline auto getCreateCube(const std::shared_ptr<GameManager> &gameManager,
                          const std::shared_ptr<Shader> &shader) {
  return [&](glm::vec3 position, glm::vec3 color, float size = 0.1f,
             const std::string &name = "") {
    Material cubeMaterial;
    cubeMaterial.type = MaterialType::UNIFORM;
    cubeMaterial.ambient = color * 0.2f;
    cubeMaterial.diffuse = color * 0.8f;
    cubeMaterial.specular = glm::vec3(0.1f);
    cubeMaterial.shininess = 32.0f;

    auto cube = ModelFactory::createCube(size, cubeMaterial, name);
    auto cubeObj = std::make_unique<GameObject>(std::move(cube), shader, name);
    cubeObj->position = position;

    return gameManager->addObject(std::move(cubeObj));
  };
}
