#pragma once

#include <glad/glad.h>
#include <vector>

namespace GLUtils {
    /**
     * Creates a Vertex Array Object with the specified vertex data
     * @param vertices Vector of vertex data
     * @param vertexSize Size of each vertex in floats
     * @return Pair of (VAO, VBO) IDs
     */
    std::pair<unsigned int, unsigned int> createVAO(const std::vector<float>& vertices, size_t vertexSize);

    /**
     * Creates a Vertex Array Object with the specified vertex data (raw array)
     * @param vertices Array of vertex data
     * @param vertexCount Number of vertices
     * @param vertexSize Size of each vertex in floats
     * @return Pair of (VAO, VBO) IDs
     */
    std::pair<unsigned int, unsigned int> createVAO(const float* vertices, size_t vertexCount, size_t vertexSize);

    /**
     * Deletes a VAO and its associated VBO
     * @param vao VAO ID to delete
     * @param vbo VBO ID to delete
     */
    void deleteVAO(unsigned int vao, unsigned int vbo);
}