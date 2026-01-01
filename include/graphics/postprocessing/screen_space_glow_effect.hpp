#pragma once

#include <epoxy/gl.h>
#include <glm/glm.hpp>
#include <memory>
#include <string>

#include "graphics/shader.h"
#include "math/transform.hpp"
#include "post_processing_effect.hpp"
#include "screen_quad.hpp"
#include "scene/game_object.hpp"

namespace graphics::postprocessing {

/// Base class for screen-space glow effects
/// Provides common functionality for glow effects
class ScreenSpaceGlowEffect : public PostProcessingEffect {
public:
  ScreenSpaceGlowEffect() = default;
  virtual ~ScreenSpaceGlowEffect() override { cleanup(); }

  bool initialize() override {
    if (initialized) {
      return true;
    }

    try {
      shader = std::make_shared<Shader>("shaders/screen-quad.vs.glsl",
                                        getShaderPath().c_str());

      {
        screenQuad = std::make_unique<ScreenQuad>();
        if (not screenQuad->initialize()) {
          throw std::runtime_error{"Failed to initialize screen quad"};
        }
      }

      initialized = true;
      active = false; // Glow effects start inactive

      std::println(std::clog, "{} initialized", getName());
      return true;
    } catch (const std::exception &e) {
      std::println(std::cerr, "Failed to initialize {}: {}", getName(),
                   e.what());
      cleanup();
      return false;
    }
  }

  void cleanup() override {
    shader.reset();
    screenQuad.reset();
    activeGlowObject = nullptr;
    effectStrength = 0.0f;
    effectTime = 0.0f;
    initialized = false;
    active = false;
  }

  void apply(GLuint inputTexture, GLuint targetFBO,
             const glm::vec2 &screenSize) override {
    if (not initialized or not active or not shader or not screenQuad or
        not activeGlowObject) {
      return;
    }

    // Glow position in screen space
    const auto glowScreenPos = calculateGlowScreenPos();

    // Bind FBO
    glBindFramebuffer(GL_FRAMEBUFFER, targetFBO);
    glClear(GL_COLOR_BUFFER_BIT);

    shader->use();

    {
      shader->setVec2("screenSize", screenSize);
      shader->setVec2("glowScreenPos", glowScreenPos);
      shader->setFloat("effectStrength", effectStrength);
      shader->setFloat("time", effectTime);
    }

    setGlowUniforms();

    { // Texture
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, inputTexture);
      shader->setInt("sceneTexture", 0);
    }

    screenQuad->render();

    // Unbind
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  void update(float deltaTime) override {
    if (not active or not activeGlowObject) {
      return;
    }

    effectTime += deltaTime;
    effectStrength = 0.8f + 0.2f * sin(effectTime * 2.0f);
  }

  bool isActive() const override {
    return active and initialized and activeGlowObject;
  }

  void setActive(bool newActive) override { active = newActive; }

  /// Enable glow for a specific GameObject
  void enableForObject(GameObject *obj, float initialStrength = 1.0f) {
    if (!obj) {
      std::println(std::cerr, "{}: Cannot enable with null object", getName());
      return;
    }

    activeGlowObject = obj;
    effectStrength = initialStrength;
    effectTime = 0.0f;
    active = true;

    std::println(std::clog, "{} enabled for '{}' with strength: {}", getName(),
                 obj->getName(), initialStrength);
  }

  /// Disable glow
  void disable() {
    active = false;
    effectStrength = 0.0f;
    activeGlowObject = nullptr;

    std::println(std::clog, "{} disabled", getName());
  }

  /// Get current effect strength
  float getEffectStrength() const { return effectStrength; }

  /// Get active GameObject
  GameObject *getActiveObject() const { return activeGlowObject; }

  // Glow control methods

  void setGlowColor(const glm::vec3 &color) {
    if (shader and initialized) {
      shader->use();
      shader->setVec3("glowColor", color);
    }
  }

  void setGlowRadius(float radius) {
    if (shader and initialized) {
      shader->use();
      shader->setFloat("glowRadius", radius);
    }
  }

  void setGlowIntensity(float intensity) {
    if (shader and initialized) {
      shader->use();
      shader->setFloat("glowIntensity", intensity);
    }
  }

  void setGlowFalloffMultiplier(float multiplier) {
    if (shader and initialized) {
      shader->use();
      shader->setFloat("glowFalloffMultiplier", multiplier);
    }
  }

  void setPulseBaseIntensity(float intensity) {
    if (shader and initialized) {
      shader->use();
      shader->setFloat("pulseBaseIntensity", intensity);
    }
  }

  void setPulseFrequency(float frequency) {
    if (shader and initialized) {
      shader->use();
      shader->setFloat("pulseFrequency", frequency);
    }
  }

  /// Set projection and view matrices for screen space calculation
  void setViewMatrices(const glm::mat4 &proj, const glm::mat4 &viewMat) {
    projection = proj;
    view = viewMat;
  }

protected:
  std::shared_ptr<Shader> shader;
  std::unique_ptr<ScreenQuad> screenQuad;

  // Glow effect state
  GameObject *activeGlowObject{nullptr};
  float effectStrength{0.0f};
  float effectTime{0.0f};
  bool initialized{false};
  bool active{false};

  /// Projection matrix to calculate screen position
  glm::mat4 projection;
  /// View matrix to calculate screen position
  glm::mat4 view;

  /// Calculate glow object position in screen space (0-1 range)
  glm::vec2 calculateGlowScreenPos() const {
    if (!activeGlowObject) {
      // Fallback to center
      return glm::vec2{0.5f, 0.5f};
    }

    return math::worldPosToScreen(projection, view, activeGlowObject->position);
  }

  /// Get shader path (subclasses can override)
  virtual std::string getShaderPath() const { return "shaders/glow.fs.glsl"; };

  /// Set glow-specific uniforms (subclasses can override)
  virtual void setGlowUniforms() {
    // Base implementation does nothing
    // Subclasses can set additional uniforms if needed
  }
};

} // namespace graphics::postprocessing
