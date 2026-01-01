#pragma once

#include <epoxy/gl.h>
#include <glm/glm.hpp>
#include <iostream>
#include <print>
#include <stdexcept>

namespace graphics::postprocessing {

/// Full screen quad for post-processing
/// Renders a quad covering the entire screen with texture coordinates
class ScreenQuad {
public:
  ScreenQuad() = default;
  ~ScreenQuad() { cleanup(); }

  // Disable copying
  ScreenQuad(const ScreenQuad &) = delete;
  ScreenQuad &operator=(const ScreenQuad &) = delete;

  /// Initialize the screen quad
  bool initialize() {
    if (initialized) {
      return true;
    }

    try {
      // Vertex data for a full-screen quad
      // Positions (x, y) and texture coordinates (u, v)
      float vertices[] = {
          // Positions   // TexCoords
          -1.0f, 1.0f,  0.0f, 1.0f, // Top-left
          -1.0f, -1.0f, 0.0f, 0.0f, // Bottom-left
          1.0f,  -1.0f, 1.0f, 0.0f, // Bottom-right

          -1.0f, 1.0f,  0.0f, 1.0f, // Top-left
          1.0f,  -1.0f, 1.0f, 0.0f, // Bottom-right
          1.0f,  1.0f,  1.0f, 1.0f  // Top-right
      };

      glGenVertexArrays(1, &vao);
      glGenBuffers(1, &vbo);

      glBindVertexArray(vao);
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

      // Position attribute
      glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                            (void *)0);
      glEnableVertexAttribArray(0);

      // Texture coordinate attribute
      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                            (void *)(2 * sizeof(float)));
      glEnableVertexAttribArray(1);

      glBindVertexArray(0);
      initialized = true;

      std::println(std::clog, "ScreenQuad initialized");
      return true;
    } catch (const std::exception &e) {
      std::println(std::cerr, "Failed to initialize ScreenQuad: {}", e.what());
      cleanup();
      return false;
    }
  }

  /// Clean up OpenGL resources
  void cleanup() {
    if (vbo != 0) {
      glDeleteBuffers(1, &vbo);
      vbo = 0;
    }

    if (vao != 0) {
      glDeleteVertexArrays(1, &vao);
      vao = 0;
    }

    initialized = false;
  }

  /// Render the full-screen quad
  void render() const {
    if (!initialized) {
      throw std::runtime_error{"ScreenQuad not initialized"};
    }

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
  }

private:
  GLuint vao{0};
  GLuint vbo{0};
  bool initialized{false};
};
} // namespace graphics::postprocessing
