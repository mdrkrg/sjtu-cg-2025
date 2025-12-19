#pragma once

#include <glm/glm.hpp>

enum class MaterialType {
  TEXTURED, /// Uses texture maps from model
  UNIFORM,  /// Uses uniform colors (no textures)
  EMISSIVE  /// Emits light (can be combined with TEXTURED or UNIFORM)
};

struct Material {
  MaterialType type = MaterialType::TEXTURED;

  // Color properties (used by UNIFORM type, multipliers for TEXTURED)
  glm::vec3 ambient{0.2f};
  glm::vec3 diffuse{0.8f};
  glm::vec3 specular{0.1f};
  float shininess{32.0f};

  // Emission properties (used by EMISSIVE type)
  bool emissive{false};
  glm::vec3 emissionColor{1.0f, 1.0f, 1.0f};
  float emissionStrength{1.0f};

  /// Constructor for uniform material
  Material(const glm::vec3 &ambient, const glm::vec3 &diffuse,
           const glm::vec3 &specular, float shininess = 32.0f)
      : type(MaterialType::UNIFORM), ambient(ambient), diffuse(diffuse),
        specular(specular), shininess(shininess) {}

  /// Constructor for textured material (with optional color multipliers)
  Material(float shininess = 32.0f)
      : type(MaterialType::TEXTURED), shininess(shininess) {}

  /// Enable emission
  void setEmissive(bool enabled, const glm::vec3 &color = glm::vec3(1.0f),
                   float strength = 1.0f) {
    emissive = enabled;
    emissionColor = color;
    emissionStrength = strength;
    if (enabled) {
      type = MaterialType::EMISSIVE;
    }
  }

  /// Check if material uses textures
  bool usesTextures() const { return type == MaterialType::TEXTURED; }

  /// Check if material uses uniform colors
  bool usesUniformColors() const { return type == MaterialType::UNIFORM; }

  /// Check if material is emissive
  bool isEmissive() const { return emissive; }
};
