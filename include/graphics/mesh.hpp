#pragma once

#include <epoxy/gl.h>
#include <epoxy/glx.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <graphics/shader.h>
#include <scene/aabb.hpp>

#include <string>
#include <vector>
#include <optional>
#include "math/intersection.hpp"

struct MeshVertex {
  // position
  glm::vec3 Position;
  // normal
  glm::vec3 Normal;
  // texCoords
  glm::vec2 TexCoords;
  // tangent
  glm::vec3 Tangent;
  // bitangent
  glm::vec3 Bitangent;
};

struct Texture {
  unsigned int id;
  std::string type;
  std::string path;
};

class Mesh {
public:
  // mesh Data
  std::vector<MeshVertex> vertices;
  std::vector<unsigned int> indices;
  std::vector<Texture> textures;
  unsigned int VAO;

  // Name of the material in .mtl file
  std::string name;

  // constructor
  Mesh(std::vector<MeshVertex> &&vertices, std::vector<unsigned int> &&indices,
       std::vector<Texture> &&textures, const std::string &name = "")
      : vertices{vertices}, indices{indices}, textures{textures}, name{name} {

    // now that we have all the required data, set the vertex buffers and its
    // attribute pointers.
    setupMesh();
  }

  /// Create a cube mesh
  static Mesh createCube(float size = 1.0f, const std::string &name = "") {
    std::vector<MeshVertex> vertices;
    std::vector<unsigned int> indices;
    float s = size * 0.5f;
    // 8 vertices
    glm::vec3 positions[8] = {{-s, -s, s}, {s, -s, s},   {s, s, s},
                              {-s, s, s},  {-s, -s, -s}, {s, -s, -s},
                              {s, s, -s},  {-s, s, -s}};
    glm::vec3 normals[6] = {{0, 0, 1},  {0, 0, -1}, {1, 0, 0},
                            {-1, 0, 0}, {0, 1, 0},  {0, -1, 0}};
    glm::vec2 texCoords[4] = {
        {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};
    unsigned int faceVertices[6][4] = {{0, 1, 2, 3}, {5, 4, 7, 6},
                                       {1, 5, 6, 2}, {4, 0, 3, 7},
                                       {3, 2, 6, 7}, {4, 5, 1, 0}};
    for (int face = 0; face < 6; ++face) {
      unsigned int baseIndex = vertices.size();
      for (int i = 0; i < 4; ++i) {
        MeshVertex v;
        v.Position = positions[faceVertices[face][i]];
        v.Normal = normals[face];
        v.TexCoords = texCoords[i];
        v.Tangent = glm::vec3(0.0f);
        v.Bitangent = glm::vec3(0.0f);
        vertices.push_back(v);
      }
      indices.push_back(baseIndex);
      indices.push_back(baseIndex + 1);
      indices.push_back(baseIndex + 2);
      indices.push_back(baseIndex);
      indices.push_back(baseIndex + 2);
      indices.push_back(baseIndex + 3);
    }
    std::vector<Texture> textures;
    return Mesh(std::move(vertices), std::move(indices), std::move(textures),
                name);
  }

  /// Create a sphere mesh
  static Mesh createSphere(float radius = 1.0f, int sectors = 36,
                           int stacks = 18, const std::string &name = "") {
    std::vector<MeshVertex> vertices;
    std::vector<unsigned int> indices;

    const float PI = glm::pi<float>();
    float sectorStep = 2.0f * PI / sectors;
    float stackStep = PI / stacks;

    // Generate vertices
    for (int i = 0; i <= stacks; ++i) {
      float stackAngle = PI / 2.0f - i * stackStep;
      float xy = radius * std::cosf(stackAngle);
      float y = radius * std::sinf(stackAngle);

      for (int j = 0; j <= sectors; ++j) {
        float sectorAngle = j * sectorStep;

        float x = xy * cosf(sectorAngle);
        float z = xy * sinf(sectorAngle);

        MeshVertex vertex;
        vertex.Position = glm::vec3(x, y, z);
        vertex.Normal = glm::normalize(vertex.Position);
        vertex.TexCoords = glm::vec2((float)j / sectors, (float)i / stacks);
        vertex.Tangent = glm::vec3(0.0f);
        vertex.Bitangent = glm::vec3(0.0f);
        vertices.push_back(vertex);
      }
    }

    // Generate indices
    for (int i = 0; i < stacks; ++i) {
      for (int j = 0; j < sectors; ++j) {
        int first = i * (sectors + 1) + j;
        int second = first + sectors + 1;

        // First triangle
        indices.push_back(first);
        indices.push_back(second);
        indices.push_back(first + 1);

        // Second triangle
        indices.push_back(first + 1);
        indices.push_back(second);
        indices.push_back(second + 1);
      }
    }

    std::vector<Texture> textures;
    return Mesh(std::move(vertices), std::move(indices), std::move(textures),
                name);
  }

  /// Bind all textures of the mesh to the shader
  void bindTextures(const Shader &shader) const {
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    unsigned int normalNr = 1;
    unsigned int heightNr = 1;
    for (unsigned int i = 0; i < textures.size(); i++) {
      glActiveTexture(GL_TEXTURE0 + i);
      std::string number;
      std::string name = textures[i].type;
      if (name == "texture_diffuse")
        number = std::to_string(diffuseNr++);
      else if (name == "texture_specular")
        number = std::to_string(specularNr++);
      else if (name == "texture_normal")
        number = std::to_string(normalNr++);
      else if (name == "texture_height")
        number = std::to_string(heightNr++);

      glUniform1i(glGetUniformLocation(shader.ID, (name + number).c_str()), i);
      glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }
  }

  // render the mesh
  void Draw(const Shader &shader) const {
    // bind appropriate textures
    bindTextures(shader);

    // draw mesh
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()),
                   GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // always good practice to set everything back to defaults once configured.
    glActiveTexture(GL_TEXTURE0);
  }

  /// Render the mesh with instanced rendering
  void DrawInstanced(const Shader &shader, unsigned int instanceCount) const {
    // bind appropriate textures
    bindTextures(shader);

    // draw mesh instanced
    glBindVertexArray(VAO);
    glDrawElementsInstanced(GL_TRIANGLES,
                            static_cast<unsigned int>(indices.size()),
                            GL_UNSIGNED_INT, 0, instanceCount);
    glBindVertexArray(0);

    // reset texture unit
    glActiveTexture(GL_TEXTURE0);
  }

  /// Draw mesh with external instance buffer (for systems that manage their own
  /// instance VBO)
  /// @param shader Shader to use
  /// @param instanceVBO External instance buffer object ID
  /// @param instanceCount Number of instances to draw
  /// @param baseAttributeLocation Starting attribute location for instance
  /// matrix (default: 3)
  void drawWithExternalInstanceBuffer(const Shader &shader,
                                      unsigned int instanceVBO,
                                      unsigned int instanceCount,
                                      size_t baseAttributeLocation = 3) const {
    // bind appropriate textures
    bindTextures(shader);

    // bind mesh VAO
    glBindVertexArray(VAO);

    // bind external instance VBO and set up instance attributes
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);

    // mat4 takes 4 attribute locations
    for (int i = 0; i < 4; ++i) {
      glEnableVertexAttribArray(baseAttributeLocation + i);
      glVertexAttribPointer(baseAttributeLocation + i, 4, GL_FLOAT, GL_FALSE,
                            sizeof(glm::mat4), (void *)(i * sizeof(glm::vec4)));
      glVertexAttribDivisor(baseAttributeLocation + i, 1);
    }

    // draw instanced
    glDrawElementsInstanced(GL_TRIANGLES,
                            static_cast<unsigned int>(indices.size()),
                            GL_UNSIGNED_INT, 0, instanceCount);

    // cleanup instance attributes
    for (int i = 0; i < 4; ++i) {
      glDisableVertexAttribArray(baseAttributeLocation + i);
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // reset texture unit
    glActiveTexture(GL_TEXTURE0);
  }

  /// Get the axis-aligned bounding box of the mesh in local space
  /// @return AABB with min and max points
  scene::AABB getLocalAABB() const {
    if (vertices.empty()) {
      return scene::AABB{glm::vec3{0.0f}, glm::vec3{0.0f}};
    }

    glm::vec3 min = vertices[0].Position;
    glm::vec3 max = vertices[0].Position;

    for (const auto &vertex : vertices) {
      min = glm::min(min, vertex.Position);
      max = glm::max(max, vertex.Position);
    }

    return scene::AABB{min, max};
  }

  /// Get the axis-aligned bounding box of the mesh in world space
  /// @param modelMatrix The model matrix to transform the mesh to world space
  /// @return AABB with min and max points in world space
  scene::AABB getWorldAABB(const glm::mat4 &modelMatrix) const {
    scene::AABB localAABB = getLocalAABB();
    return localAABB.transform(modelMatrix);
  }

  /// Check if a point is inside the mesh using ray casting parity test
  /// @param point The point to test in world space
  /// @param modelMatrix The model matrix to transform the mesh to world space
  /// @return true if the point is inside the mesh, false otherwise
  bool containsPoint(const glm::vec3 &point,
                     const glm::mat4 &modelMatrix) const;

  /// Perform ray-mesh intersection test
  /// @param ray The ray in world space (normalized)
  /// @param modelMatrix The model matrix to transform the mesh to world space
  /// @return Optional hit information with distance, point, and normal
  std::optional<math::RayHit>
  rayIntersection(const math::Ray &ray, const glm::mat4 &modelMatrix) const;

private:
  // render data
  unsigned int VBO, EBO;

  // initializes all the buffer objects/arrays
  void setupMesh() {
    // create buffers/arrays
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    // load data into vertex buffers
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // A great thing about structs is that their memory layout is sequential for
    // all its items. The effect is that we can simply pass a pointer to the
    // struct and it translates perfectly to a glm::vec3/2 array which again
    // translates to 3/2 floats which translates to a byte array.
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(MeshVertex),
                 &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
                 &indices[0], GL_STATIC_DRAW);

    // set the vertex attribute pointers
    // vertex Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex),
                          (void *)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex),
                          (void *)offsetof(MeshVertex, Normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(MeshVertex),
                          (void *)offsetof(MeshVertex, TexCoords));
    // vertex tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex),
                          (void *)offsetof(MeshVertex, Tangent));
    // vertex bitangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex),
                          (void *)offsetof(MeshVertex, Bitangent));
  }
};
