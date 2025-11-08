#include "gl_utils.h"

namespace GLUtils {

std::pair<unsigned int, unsigned int>
createVAO(const std::vector<float> &vertices, size_t vertexSize) {
  unsigned int VAO, VBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
               vertices.data(), GL_STATIC_DRAW);

  // Position attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vertexSize * sizeof(float),
                        (void *)0);
  glEnableVertexAttribArray(0);

  glBindVertexArray(0);
  return std::make_pair(VAO, VBO);
}

std::pair<unsigned int, unsigned int>
createVAO(const float *vertices, size_t vertexCount, size_t vertexSize) {
  unsigned int VAO, VBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(float), vertices,
               GL_STATIC_DRAW);

  // Position attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vertexSize * sizeof(float),
                        (void *)0);
  glEnableVertexAttribArray(0);

  glBindVertexArray(0);
  return std::make_pair(VAO, VBO);
}

void deleteVAO(unsigned int vao, unsigned int vbo) {
  if (vao != 0) {
    glDeleteVertexArrays(1, &vao);
  }
  if (vbo != 0) {
    glDeleteBuffers(1, &vbo);
  }
}
} // namespace GLUtils
