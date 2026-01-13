#pragma once

#include "model.hpp"
#include "material.hpp"

#include <epoxy/gl.h>
#include <glm/glm.hpp>
#include <stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>
#include <vector>
#include <memory>

struct ModelWithMaterials {
  std::shared_ptr<Model> model;
  std::vector<Material> materials; // One per mesh in the model

  ModelWithMaterials(std::shared_ptr<Model> model,
                     std::vector<Material> materials = {})
      : model(std::move(model)), materials(std::move(materials)) {}

  // Copy constructor
  ModelWithMaterials(const ModelWithMaterials &other) = default;

  // Move constructor
  ModelWithMaterials(ModelWithMaterials &&other) noexcept = default;

  // Copy assignment
  ModelWithMaterials &operator=(const ModelWithMaterials &other) = default;

  // Move assignment
  ModelWithMaterials &operator=(ModelWithMaterials &&other) noexcept = default;
};

class ModelFactory {
public:
  /// Load model from file using Assimp
  static ModelWithMaterials loadModel(const std::string &path);

  /// Create from programmatic meshes with explicit materials
  static ModelWithMaterials createFromMeshes(std::vector<Mesh> &&meshes,
                                             std::vector<Material> &&materials);

  /// Create cube with material
  static ModelWithMaterials createCube(float size = 1.0f,
                                       const Material &material = Material(),
                                       const std::string &name = "");

  /// Create sphere with material
  static ModelWithMaterials createSphere(float radius = 1.0f,
                                         const Material &material = Material(),
                                         const std::string &name = "");

  /// Create simple wall (quad)
  /// @param size Wall dimensions (width, height, thickness)
  /// @param material Wall material
  /// @param name Model name
  static ModelWithMaterials
  createWall(const glm::vec3 &size = glm::vec3(2.0f, 1.0f, 0.1f),
             const Material &material = Material(),
             const std::string &name = "");

private:
  // Helper to load texture from disk (Moved from Model)
  static unsigned int TextureFromFile(const char *path,
                                      const std::string &directory);

  // Internal helper class to maintain state during recursive Assimp loading
  struct LoaderContext {
    std::string directory;
    std::vector<Texture> textures_loaded;
    std::vector<Mesh> meshes;
    std::vector<Material> materials;

    void processNode(aiNode *node, const aiScene *scene);
    Mesh processMesh(aiMesh *mesh, const aiScene *scene);
    std::vector<Texture> loadMaterialTextures(aiMaterial *mat,
                                              aiTextureType type,
                                              std::string typeName);
  };
};

inline ModelWithMaterials ModelFactory::loadModel(const std::string &path) {
  Assimp::Importer importer;
  const aiScene *scene = importer.ReadFile(
      path, aiProcess_Triangulate | aiProcess_GenSmoothNormals |
                aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !scene->mRootNode) {
    std::println(std::cerr, "ERROR::ASSIMP:: {}", importer.GetErrorString());
    // Return empty model on failure
    return createFromMeshes({}, {});
  }

  LoaderContext context{};
  context.directory = path.substr(0, path.find_last_of('/'));

  // Start recursion
  context.processNode(scene->mRootNode, scene);

  // Construct final Model object
  auto model = std::make_shared<Model>(std::move(context.meshes),
                                       std::move(context.textures_loaded));

  return {std::move(model), std::move(context.materials)};
}

inline ModelWithMaterials
ModelFactory::createFromMeshes(std::vector<Mesh> &&meshes,
                               std::vector<Material> &&materials) {
  auto model = std::make_shared<Model>(std::move(meshes));
  return {std::move(model), std::move(materials)};
}

inline ModelWithMaterials ModelFactory::createCube(float size,
                                                   const Material &material,
                                                   const std::string &name) {
  auto mesh = Mesh::createCube(size, name);
  std::vector<Mesh> meshes;
  meshes.push_back(std::move(mesh));

  std::vector<Material> materials;
  materials.push_back(material);

  return createFromMeshes(std::move(meshes), std::move(materials));
}

inline ModelWithMaterials ModelFactory::createSphere(float radius,
                                                     const Material &material,
                                                     const std::string &name) {
  auto mesh = Mesh::createSphere(radius, 36, 18, name);
  std::vector<Mesh> meshes;
  meshes.push_back(std::move(mesh));

  std::vector<Material> materials;
  materials.push_back(material);

  return createFromMeshes(std::move(meshes), std::move(materials));
}

inline ModelWithMaterials ModelFactory::createWall(const glm::vec3 &size,
                                                   const Material &material,
                                                   const std::string &name) {
  auto mesh = Mesh::createCube(1.0f, name.empty() ? "wall" : name);

  // Scale the mesh to the desired wall size
  // We need to transform vertices to match wall dimensions
  std::vector<Vertex> scaledVertices;
  for (const auto &vertex : mesh.vertices) {
    Vertex scaledVertex = vertex;
    scaledVertex.Position.x *= size.x;
    scaledVertex.Position.y *= size.y;
    scaledVertex.Position.z *= size.z;

    // Scale texture coordinates based on wall dimensions for repeating textures
    // Front/back: scale by width (x) and height (y)
    // Left/right: scale by depth (z) and height (y)
    // Top/bottom: scale by width (x) and depth (z)

    // Determine which face this vertex belongs to based on its normal
    if (glm::abs(scaledVertex.Normal.z) > 0.5f) {
      // Front/back face
      scaledVertex.TexCoords.x *= size.x;
      scaledVertex.TexCoords.y *= size.y;
    } else if (glm::abs(scaledVertex.Normal.x) > 0.5f) {
      // Left/right face
      scaledVertex.TexCoords.x *= size.z;
      scaledVertex.TexCoords.y *= size.y;
    } else {
      // Top/bottom
      scaledVertex.TexCoords.x *= size.x;
      scaledVertex.TexCoords.y *= size.z;
    }

    scaledVertices.push_back(scaledVertex);
  }

  // Create new mesh with scaled vertices
  Mesh scaledMesh(std::move(scaledVertices), std::move(mesh.indices),
                  std::move(mesh.textures), mesh.name);

  std::vector<Mesh> meshes;
  meshes.push_back(std::move(scaledMesh));

  std::vector<Material> materials;
  materials.push_back(material);

  return createFromMeshes(std::move(meshes), std::move(materials));
}

inline void ModelFactory::LoaderContext::processNode(aiNode *node,
                                                     const aiScene *scene) {
  // Process each mesh located at the current node
  for (unsigned int i = 0; i < node->mNumMeshes; i++) {
    aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
    meshes.push_back(processMesh(mesh, scene));
  }
  // Recursively process children
  for (unsigned int i = 0; i < node->mNumChildren; i++) {
    processNode(node->mChildren[i], scene);
  }
}

inline Mesh ModelFactory::LoaderContext::processMesh(aiMesh *mesh,
                                                     const aiScene *scene) {
  std::vector<MeshVertex> vertices;
  std::vector<unsigned int> indices;
  std::vector<Texture> textures;

  // 1. Process Vertices
  for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
    MeshVertex vertex;
    glm::vec3 vector;

    // Positions
    vector.x = mesh->mVertices[i].x;
    vector.y = mesh->mVertices[i].y;
    vector.z = mesh->mVertices[i].z;
    vertex.Position = vector;

    // Normals
    if (mesh->HasNormals()) {
      vector.x = mesh->mNormals[i].x;
      vector.y = mesh->mNormals[i].y;
      vector.z = mesh->mNormals[i].z;
      vertex.Normal = vector;
    }

    // Texture Coordinates
    if (mesh->mTextureCoords[0]) {
      glm::vec2 vec;
      vec.x = mesh->mTextureCoords[0][i].x;
      vec.y = mesh->mTextureCoords[0][i].y;
      vertex.TexCoords = vec;

      // Tangent
      vector.x = mesh->mTangents[i].x;
      vector.y = mesh->mTangents[i].y;
      vector.z = mesh->mTangents[i].z;
      vertex.Tangent = vector;

      // Bitangent
      vector.x = mesh->mBitangents[i].x;
      vector.y = mesh->mBitangents[i].y;
      vector.z = mesh->mBitangents[i].z;
      vertex.Bitangent = vector;
    } else {
      vertex.TexCoords = glm::vec2(0.0f, 0.0f);
    }
    vertices.push_back(vertex);
  }

  // 2. Process Indices
  for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
    aiFace face = mesh->mFaces[i];
    for (unsigned int j = 0; j < face.mNumIndices; j++)
      indices.push_back(face.mIndices[j]);
  }

  // 3. Process Materials & Textures
  aiMaterial *aiMat = scene->mMaterials[mesh->mMaterialIndex];

  // Extract Material properties
  Material matData{};

  aiString matName{};
  aiMat->Get(AI_MATKEY_NAME, matName);
  std::string materialName = std::string(matName.C_Str());

  auto getColor = [](aiMaterial *m, const char *key, unsigned int type,
                     unsigned int idx) {
    aiColor3D c(0.f, 0.f, 0.f);
    m->Get(key, type, idx, c);
    return glm::vec3(c.r, c.g, c.b);
  };

  matData.ambient = getColor(aiMat, AI_MATKEY_COLOR_AMBIENT);
  matData.diffuse = getColor(aiMat, AI_MATKEY_COLOR_DIFFUSE);
  matData.specular = getColor(aiMat, AI_MATKEY_COLOR_SPECULAR);
  aiMat->Get(AI_MATKEY_SHININESS, matData.shininess);

  // Load Textures for the GPU Mesh
  std::vector<Texture> diffuseMaps =
      loadMaterialTextures(aiMat, aiTextureType_DIFFUSE, "texture_diffuse");
  textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

  std::vector<Texture> specularMaps =
      loadMaterialTextures(aiMat, aiTextureType_SPECULAR, "texture_specular");
  textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

  std::vector<Texture> normalMaps =
      loadMaterialTextures(aiMat, aiTextureType_HEIGHT, "texture_normal");
  textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

  std::vector<Texture> heightMaps =
      loadMaterialTextures(aiMat, aiTextureType_AMBIENT, "texture_height");
  textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

  // Determine Material Type based on loaded textures
  if (!textures.empty()) {
    matData.type = MaterialType::TEXTURED;
  } else {
    matData.type = MaterialType::UNIFORM;
  }

  // Save the material to the context list
  materials.push_back(matData);

  return Mesh(std::move(vertices), std::move(indices), std::move(textures),
              materialName);
}

inline std::vector<Texture> ModelFactory::LoaderContext::loadMaterialTextures(
    aiMaterial *mat, aiTextureType type, std::string typeName) {
  std::vector<Texture> textures;
  for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
    aiString str{};
    mat->GetTexture(type, i, &str);

    bool skip = false;
    for (unsigned int j = 0; j < textures_loaded.size(); j++) {
      if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0) {
        textures.push_back(textures_loaded[j]);
        skip = true;
        break;
      }
    }

    if (!skip) {
      Texture texture{};
      texture.id = ModelFactory::TextureFromFile(str.C_Str(), this->directory);
      texture.type = typeName;
      texture.path = str.C_Str();
      textures.push_back(texture);
      // Cache it to prevent reloading
      textures_loaded.push_back(texture);
    }
  }
  return textures;
}

inline unsigned int
ModelFactory::TextureFromFile(const char *path, const std::string &directory) {
  std::string filename{path};
  filename = directory + '/' + filename;

  unsigned int textureID;
  glGenTextures(1, &textureID);

  int width, height, nrComponents;
  unsigned char *data =
      stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
  if (data) {
    GLenum format = GL_RED; // Default init
    if (nrComponents == 1)
      format = GL_RED;
    else if (nrComponents == 3)
      format = GL_RGB;
    else if (nrComponents == 4)
      format = GL_RGBA;

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
  } else {
    std::println(std::cerr, "Texture failed to load at path: {}", filename);
    stbi_image_free(data);
  }

  return textureID;
}
