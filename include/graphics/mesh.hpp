#pragma once

#include <epoxy/gl.h>
#include <epoxy/glx.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <graphics/shader.h>

#include <string>
#include <vector>

#define MAX_BONE_INFLUENCE 4

struct Vertex {
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
  // bone indexes which will influence this vertex
  int m_BoneIDs[MAX_BONE_INFLUENCE];
  // weights from each bone
  float m_Weights[MAX_BONE_INFLUENCE];
};

struct Texture {
  unsigned int id;
  std::string type;
  std::string path;
};

class Mesh {
public:
  // mesh Data
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  std::vector<Texture> textures;
  unsigned int VAO;

  // Name of the material in .mtl file
  std::string name;

  // constructor
  Mesh(std::vector<Vertex> &&vertices, std::vector<unsigned int> &&indices,
       std::vector<Texture> &&textures, const std::string &name = "")
      : vertices{vertices}, indices{indices}, textures{textures}, name{name} {

    // now that we have all the required data, set the vertex buffers and its
    // attribute pointers.
    setupMesh();
  }

  /// Create a cube mesh
  static Mesh createCube(float size = 1.0f, const std::string &name = "") {
    std::vector<Vertex> vertices;
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
        Vertex v;
        v.Position = positions[faceVertices[face][i]];
        v.Normal = normals[face];
        v.TexCoords = texCoords[i];
        v.Tangent = glm::vec3(0.0f);
        v.Bitangent = glm::vec3(0.0f);
        for (int j = 0; j < MAX_BONE_INFLUENCE; ++j) {
          v.m_BoneIDs[j] = 0;
          v.m_Weights[j] = 0.0f;
        }
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

  // render the mesh
  void Draw(const Shader &shader) const {
    // bind appropriate textures
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    unsigned int normalNr = 1;
    unsigned int heightNr = 1;
    for (unsigned int i = 0; i < textures.size(); i++) {
      glActiveTexture(GL_TEXTURE0 +
                      i); // active proper texture unit before binding
      // retrieve texture number (the N in diffuse_textureN)
      std::string number;
      std::string name = textures[i].type;
      if (name == "texture_diffuse")
        number = std::to_string(diffuseNr++);
      else if (name == "texture_specular")
        number =
            std::to_string(specularNr++); // transfer unsigned int to string
      else if (name == "texture_normal")
        number = std::to_string(normalNr++); // transfer unsigned int to string
      else if (name == "texture_height")
        number = std::to_string(heightNr++); // transfer unsigned int to string

      // now set the sampler to the correct texture unit
      glUniform1i(glGetUniformLocation(shader.ID, (name + number).c_str()), i);
      // and finally bind the texture
      glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }

    // draw mesh
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()),
                   GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // always good practice to set everything back to defaults once configured.
    glActiveTexture(GL_TEXTURE0);
  }

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
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
                 &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
                 &indices[0], GL_STATIC_DRAW);

    // set the vertex attribute pointers
    // vertex Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *)offsetof(Vertex, Normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *)offsetof(Vertex, TexCoords));
    // vertex tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *)offsetof(Vertex, Tangent));
    // vertex bitangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *)offsetof(Vertex, Bitangent));
    // ids
    glEnableVertexAttribArray(5);
    glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex),
                           (void *)offsetof(Vertex, m_BoneIDs));

    // weights
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *)offsetof(Vertex, m_Weights));
    glBindVertexArray(0);
  }
};
