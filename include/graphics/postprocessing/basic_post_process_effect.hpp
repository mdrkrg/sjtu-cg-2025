#pragma once

#include <epoxy/gl.h>
#include <glm/glm.hpp>
#include <memory>
#include <string>

#include "graphics/shader.h"
#include "post_processing_effect.hpp"
#include "screen_quad.hpp"

namespace graphics::postprocessing {

/// Basic post-processing effect: tone mapping + gamma correction (sRGB)
class BasicPostProcessEffect : public PostProcessingEffect {
public:
  BasicPostProcessEffect() = default;
  ~BasicPostProcessEffect() override { cleanup(); }

  bool initialize() override {
    if (initialized) {
      return true;
    }

    try {
      shader = std::make_shared<Shader>("shaders/screen-quad.vs.glsl",
                                        "shaders/post-process.fs.glsl");

      screenQuad = std::make_unique<ScreenQuad>();
      if (not screenQuad->initialize()) {
        throw std::runtime_error{"Failed to initialize screen quad"};
      }

      initialized = true;
      active = true; // Activate default

      std::println(std::clog, "BasicPostProcessEffect initialized");
      return true;
    } catch (const std::exception &e) {
      std::println(std::cerr, "Failed to initialize BasicPostProcessEffect: {}",
                   e.what());
      cleanup();
      return false;
    }
  }

  void cleanup() override {
    shader.reset();
    screenQuad.reset();
    initialized = false;
    active = false;
  }

  void apply(GLuint inputTexture, GLuint targetFBO,
             const glm::vec2 &screenSize) override {
    if (not initialized or not active or not shader or not screenQuad) {
      return;
    }

    // Bind target FBO (0 for screen)
    glBindFramebuffer(GL_FRAMEBUFFER, targetFBO);
    glClear(GL_COLOR_BUFFER_BIT);

    // Use shader
    shader->use();

    // Set uniforms
    shader->setVec2("screenSize", screenSize);

    // Bind input texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, inputTexture);
    shader->setInt("sceneTexture", 0);

    // Render screen quad
    screenQuad->render();

    // Unbind FBO
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  void update(float deltaTime) override {
    // Basic effect doesn't have animations
    (void)deltaTime; // Unused parameter
  }

  std::string getName() const override { return "BasicPostProcessEffect"; }

  bool isActive() const override { return active and initialized; }

  void setActive(bool newActive) override { active = newActive; }

  void setExposure(float exposure) {
    if (shader and initialized) {
      shader->use();
      shader->setFloat("exposure", exposure);
    }
  }

  void setDefaultPreset() { setExposure(1.8f); }

private:
  std::shared_ptr<Shader> shader;
  std::unique_ptr<ScreenQuad> screenQuad;
  bool initialized{false};
  bool active{false};
};

} // namespace graphics::postprocessing
