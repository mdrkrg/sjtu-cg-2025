#pragma once

#include <epoxy/gl.h>
#include <glm/glm.hpp>
#include <iostream>
#include <print>
#include <stdexcept>

namespace graphics::postprocessing {

/// Frame Buffer Object with single color attachment
class FrameBuffer {
public:
  FrameBuffer(int width, int height, bool useDepthStencil = true)
      : width{width}, height{height}, useDepthStencil{useDepthStencil} {}

  ~FrameBuffer() { cleanup(); }

  FrameBuffer(const FrameBuffer &) = delete;
  FrameBuffer &operator=(const FrameBuffer &) = delete;

  /// Initialize the frame buffer
  bool initialize() {
    if (initialized) {
      return true;
    }

    try {
      glGenFramebuffers(1, &fbo);
      glBindFramebuffer(GL_FRAMEBUFFER, fbo);

      // Create color texture
      createColorTexture();

      // Create depth/stencil buffer if requested
      if (useDepthStencil) {
        createDepthStencilBuffer();
      }

      // Check framebuffer completeness
      checkStatus();

      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      initialized = true;

      std::println(std::clog, "FrameBuffer initialized: {}x{}", width,
                   height);
      return true;
    } catch (const std::exception &e) {
      std::println(std::cerr, "Failed to initialize FrameBuffer: {}",
                   e.what());
      cleanup();
      return false;
    }
  }

  /// Clean up OpenGL resources
  void cleanup() {
    if (colorTexture != 0) {
      glDeleteTextures(1, &colorTexture);
      colorTexture = 0;
    }

    if (depthStencilBuffer != 0) {
      glDeleteRenderbuffers(1, &depthStencilBuffer);
      depthStencilBuffer = 0;
    }

    if (fbo != 0) {
      glDeleteFramebuffers(1, &fbo);
      fbo = 0;
    }

    initialized = false;
  }

  /// Bind this frame buffer for rendering
  void bind() const {
    if (!initialized) {
      throw std::runtime_error{"FrameBuffer not initialized"};
    }
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, width, height);
  }

  /// Unbind (bind default frame buffer)
  void unbind() const { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

  /// Resize the frame buffer
  void resize(int newWidth, int newHeight) {
    if (newWidth == width && newHeight == height) {
      return;
    }

    width = newWidth;
    height = newHeight;

    // Recreate with new size
    cleanup();
    initialize();
  }

  // Getters
  GLuint getFBO() const { return fbo; }
  GLuint getColorTexture() const { return colorTexture; }
  int getWidth() const { return width; }
  int getHeight() const { return height; }
  bool isValid() const { return initialized; }

private:
  GLuint fbo{0};
  GLuint colorTexture{0};
  GLuint depthStencilBuffer{0};

  int width;
  int height;
  bool useDepthStencil;
  bool initialized{false};

  void createColorTexture() {
    glGenTextures(1, &colorTexture);
    glBindTexture(GL_TEXTURE_2D, colorTexture);

    // Use RGBA16F for HDR support
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA,
                 GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           colorTexture, 0);
  }

  void createDepthStencilBuffer() {
    glGenRenderbuffers(1, &depthStencilBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthStencilBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                              GL_RENDERBUFFER, depthStencilBuffer);
  }

  void checkStatus() const {
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
      throw std::runtime_error{"Framebuffer not complete"};
    }
  }
};

} // namespace graphics::postprocessing
