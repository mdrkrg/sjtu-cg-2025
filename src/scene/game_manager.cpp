#include "scene/game_manager.hpp"
#include "math/intersection.hpp"
#include <algorithm>
#include <iostream>
#include <limits>
#include <print>

GameManager::GameManager() {
  std::cout << "GameManager initialized" << std::endl;
  puzzleManager = std::make_unique<PuzzleManager>();
  puzzleManager->onComplete = [](PuzzleManager *) {
    std::println("Puzzle Solved!!!");
  };
}

GameObject *GameManager::addObject(std::unique_ptr<GameObject> obj) {
  GameObject *rawPtr = obj.get();
  objects.push_back(std::move(obj));
  updateObjectPointers();
  return rawPtr;
}

void GameManager::update(float deltaTime) {
  for (auto &obj : objects) {
    obj->update(deltaTime);
  }
}

GameObject *GameManager::handleRayCast(const glm::vec3 &rayOrigin,
                                       const glm::vec3 &rayDir) {
  GameObject *closestObject = nullptr;
  float closestDistance = std::numeric_limits<float>::max();

  for (auto &obj : objects) {
    auto aabb = obj->getWorldAABB();
    auto hit = math::raycastAABB(math::Ray{rayOrigin, rayDir}, aabb);
    if (hit) {
      const auto [near, far] = hit.value();
      if (near < closestDistance) {
        closestDistance = near;
        closestObject = obj.get();
      }
    }
  }

  // Handle selection change
  if (selectedObject != closestObject) {
    if (selectedObject) {
      selectedObject->setSelected(false);
    }
    selectedObject = closestObject;
    if (selectedObject) {
      selectedObject->setSelected(true);
      if (onObjectSelected) {
        onObjectSelected(selectedObject);
      }
    }
  }

  return selectedObject;
}

void GameManager::clear() {
  objects.clear();
  objectPointers.clear();
  selectedObject = nullptr;
  hoveredObject = nullptr;
}

GameObject *GameManager::findObjectByName(const std::string &name) const {
  auto it = std::find_if(objects.begin(), objects.end(),
                         [&name](const std::unique_ptr<GameObject> &obj) {
                           return obj->getName() == name;
                         });
  return it != objects.end() ? it->get() : nullptr;
}

void GameManager::updateObjectPointers() {
  objectPointers.clear();
  objectPointers.reserve(objects.size());
  for (auto &obj : objects) {
    objectPointers.push_back(obj.get());
  }
}
