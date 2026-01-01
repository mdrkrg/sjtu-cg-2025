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

/// Orb glow post-processing effect
/// Creates a magical glow around orb objects
class OrbGlowEffect : public PostProcessingEffect {
public:
  OrbGlowEffect() = default;
  ~OrbGlowEffect() override { cleanup(); }

  bool initialize() override {
    if (initialized) {
      return true;
    }

    try {
      shader = std::make_shared<Shader>("shaders/screen-quad.vs.glsl",
                                        "shaders/orb-glow.fs.glsl");

      {
        screenQuad = std::make_unique<ScreenQuad>();
        if (!screenQuad->initialize()) {
          throw std::runtime_error{"Failed to initialize screen quad"};
        }
      }

      initialized = true;
      active = false; // Orb glow starts inactive

      std::println(std::clog, "OrbGlowEffect initialized");
      return true;
    } catch (const std::exception &e) {
      std::println(std::cerr, "Failed to initialize OrbGlowEffect: {}",
                   e.what());
      cleanup();
      return false;
    }
  }

  void cleanup() override {
    shader.reset();
    screenQuad.reset();
    activeOrb = nullptr;
    effectStrength = 0.0f;
    effectTime = 0.0f;
    initialized = false;
    active = false;
  }

  void apply(GLuint inputTexture, GLuint targetFBO,
             const glm::vec2 &screenSize) override {
    if (not initialized or not active or not shader or not screenQuad or
        not activeOrb) {
      return;
    }

    // Orb position in screen space
    const auto orbScreenPos = calculateOrbScreenPos();

    // Bind FBO
    glBindFramebuffer(GL_FRAMEBUFFER, targetFBO);
    glClear(GL_COLOR_BUFFER_BIT);

    shader->use();

    {
      shader->setVec2("screenSize", screenSize);
      shader->setVec2("glowScreenPos", orbScreenPos);
      shader->setFloat("effectStrength", effectStrength);
      shader->setFloat("time", effectTime);
    }

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
    if (not active or not activeOrb) {
      return;
    }

    effectTime += deltaTime;
    effectStrength = 0.8f + 0.2f * sin(effectTime * 2.0f);
  }

  std::string getName() const override { return "OrbGlowEffect"; }

  bool isActive() const override { return active && initialized && activeOrb; }

  void setActive(bool newActive) override { active = newActive; }

  /// Enable orb glow for a specific GameObject
  void enableForOrb(GameObject *orb, float initialStrength = 1.0f) {
    if (!orb) {
      std::println(std::cerr, "OrbGlowEffect: Cannot enable with null orb");
      return;
    }

    activeOrb = orb;
    effectStrength = initialStrength;
    effectTime = 0.0f;
    active = true;

    std::println(std::clog, "Orb glow enabled for '{}' with strength: {}",
                 orb->getName(), initialStrength);
  }

  /// Disable orb glow
  void disable() {
    active = false;
    effectStrength = 0.0f;
    activeOrb = nullptr;

    std::println(std::clog, "Orb glow disabled");
  }

  // Additional control methods
  void setGlowColor(const glm::vec3 &color) {
    if (shader && initialized) {
      shader->use();
      shader->setVec3("glowColor", color);
    }
  }

  void setGlowRadius(float radius) {
    if (shader && initialized) {
      shader->use();
      shader->setFloat("glowRadius", radius);
    }
  }

  void setGlowIntensity(float intensity) {
    if (shader && initialized) {
      shader->use();
      shader->setFloat("glowIntensity", intensity);
    }
  }

  /// Set projection and view matrices for screen space calculation
  void setViewMatrices(const glm::mat4 &proj, const glm::mat4 &viewMat) {
    projection = proj;
    view = viewMat;
  }

private:
  std::shared_ptr<Shader> shader;
  std::unique_ptr<ScreenQuad> screenQuad;

  // Orb effect state
  GameObject *activeOrb{nullptr};
  float effectStrength{0.0f};
  float effectTime{0.0f};
  bool initialized{false};
  bool active{false};

  /// Projection matrix to calculate screen position
  glm::mat4 projection;
  /// View matrix to calculate screen position
  glm::mat4 view;

  /// Calculate orb position in screen space (0-1)
  glm::vec2 calculateOrbScreenPos() const {
    if (!activeOrb) {
      // Fallback to center
      return glm::vec2{0.5f, 0.5f};
    }

    return math::worldPosToScreen(projection, view, activeOrb->position);
  }
};

} // namespace graphics::postprocessing
