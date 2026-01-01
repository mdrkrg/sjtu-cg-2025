#pragma once

#include <epoxy/gl.h>
#include <glm/glm.hpp>
#include <string>

namespace graphics::postprocessing {

/// Interface for post-processing effects
/// Provides interface for extensible effect system
class PostProcessingEffect {
public:
  virtual ~PostProcessingEffect() = default;

  /// Initialize the effect
  virtual bool initialize() = 0;

  /// Clean up resources
  virtual void cleanup() = 0;

  /// Apply the effect to a texture
  /// @param inputTexture The texture to apply effect to
  /// @param targetFBO The FBO to render result to (0 for screen)
  /// @param screenSize Size of the screen/texture
  virtual void apply(GLuint inputTexture, GLuint targetFBO,
                     const glm::vec2 &screenSize) = 0;

  /// Update effect parameters (for animated effects)
  virtual void update(float deltaTime) = 0;

  /// Get effect name for debugging
  virtual std::string getName() const = 0;

  /// Check if effect is active/enabled
  virtual bool isActive() const = 0;

  /// Enable/disable the effect
  virtual void setActive(bool active) = 0;
};

} // namespace graphics::postprocessing
