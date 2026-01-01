#pragma once

#include <epoxy/gl.h>
#include <glm/glm.hpp>
#include <memory>
#include <print>

#include "graphics/shader.h"
#include "frame_buffer.hpp"
#include "basic_post_process_effect.hpp"
#include "screen_quad.hpp"

namespace graphics::postprocessing {

/// Manages the post-processing pipeline
/// Orchestrates frame buffers and effects in correct order
class PostProcessingManager {
public:
  PostProcessingManager() = default;
  ~PostProcessingManager() { cleanup(); }

  PostProcessingManager(const PostProcessingManager &) = delete;
  PostProcessingManager &operator=(const PostProcessingManager &) = delete;

  /// Initialize the post-processing system
  /// @param width Initial screen width
  /// @param height Initial screen height
  bool initialize(int width, int height) {
    if (initialized) {
      return true;
    }

    try {
      { // Create main frame buffer
        mainFBO = std::make_unique<FrameBuffer>(width, height);
        if (not mainFBO->initialize()) {
          throw std::runtime_error{"Failed to initialize main FBO"};
        }
      }

      { // Create basic post-processing effect
        basicEffect = std::make_unique<BasicPostProcessEffect>();
        if (not basicEffect->initialize()) {
          throw std::runtime_error{"Failed to initialize basic effect"};
        }

        basicEffect->setDefaultPreset();
      }

      { // Create fallback shader and quad for simple texture copy
        simpleCopyShader = std::make_shared<Shader>(
            "shaders/screen-quad.vs.glsl", "shaders/simple-copy.fs.glsl");

        fallbackQuad = std::make_unique<ScreenQuad>();
        if (not fallbackQuad->initialize()) {
          throw std::runtime_error{"Failed to initialize fallback quad"};
        }
      }

      initialized = true;
      std::println(std::clog, "PostProcessingManager initialized: {}x{}", width,
                   height);
      return true;
    } catch (const std::exception &e) {
      std::println(std::cerr, "Failed to initialize PostProcessingManager: {}",
                   e.what());
      cleanup();
      return false;
    }
  }

  /// Clean up all resources
  void cleanup() {
    simpleCopyShader.reset();
    fallbackQuad.reset();
    basicEffect.reset();
    mainFBO.reset();
    initialized = false;
  }

  /// Resize all resources
  void resize(int newWidth, int newHeight) {
    if (not initialized) {
      return;
    }

    if (mainFBO) {
      mainFBO->resize(newWidth, newHeight);
    }

    std::println(std::clog, "PostProcessingManager resized: {}x{}", newWidth,
                 newHeight);
  }

  /// Begin rendering to frame buffer
  void beginRender() {
    if (not initialized or not mainFBO) {
      return;
    }

    mainFBO->bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  /// End rendering to frame buffer and apply post-processing
  /// @param projection Projection matrix for screen space calculations
  /// @param view View matrix for screen space calculations
  void endRender(const glm::mat4 &projection, const glm::mat4 &view) {
    (void)projection; // Unused for now
    (void)view;       // Unused for now

    if (not initialized or not mainFBO) {
      return;
    }

    mainFBO->unbind();

    updateEffects();

    // Get screen size
    glm::vec2 screenSize{static_cast<float>(mainFBO->getWidth()),
                         static_cast<float>(mainFBO->getHeight())};

    // Apply post-processing effects in order
    applyEffects(screenSize);
  }

  /// Update all effects (for animations)
  void updateEffects() {}

  /// Check if initialized
  bool isValid() const { return initialized; }

  // Basic effect control
  void setBasicEffectActive(bool active) {
    if (basicEffect) {
      basicEffect->setActive(active);
    }
  }

  void setExposure(float exposure) {
    if (basicEffect) {
      basicEffect->setExposure(exposure);
    }
  }

private:
  std::unique_ptr<FrameBuffer> mainFBO;
  std::unique_ptr<BasicPostProcessEffect> basicEffect;
  std::shared_ptr<Shader> simpleCopyShader;
  std::unique_ptr<ScreenQuad> fallbackQuad;
  bool initialized{false};

  /// Apply all active effects in correct order
  void applyEffects(const glm::vec2 &screenSize) {
    if (not mainFBO) {
      return;
    }

    GLuint currentTexture = mainFBO->getColorTexture();
    GLuint targetFBO = 0; // Start with screen as target

    // Basic post processing
    if (basicEffect and basicEffect->isActive()) {
      basicEffect->apply(currentTexture, targetFBO, screenSize);
    } else {
      // Fallback
      renderTextureToScreen(currentTexture);
    }
  }

  /// Simple fallback: render texture directly to screen
  void renderTextureToScreen(GLuint texture) {
    if (not simpleCopyShader or not fallbackQuad) {
      return;
    }

    // Bind screen framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    simpleCopyShader->use();

    // Bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    simpleCopyShader->setInt("sceneTexture", 0);

    fallbackQuad->render();

    // Unbind
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
};

} // namespace graphics::postprocessing
