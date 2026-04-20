#include <assimp/Importer.hpp>
#include <assimp/matrix4x4.h>
#include <assimp/scene.h>
#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>
#include <vector>

#include "model.h"

// Helper function to load embedded texture from memory (for GLB files)
unsigned int loadTextureFromMemory(const aiTexture *embeddedTex) {
  if (!embeddedTex || !embeddedTex->pcData) {
    std::cout << "ERROR: Embedded texture is null" << std::endl;
    return 0;
  }

  int width, height, nrChannels;
  unsigned char *data = nullptr;
  bool stbiOwned = false;

  if (embeddedTex->mHeight == 0) {
    // Compressed format (PNG, JPG, etc.) stored as a blob
    // Force 4 channels (RGBA) for consistency
    data = stbi_load_from_memory(
        reinterpret_cast<const unsigned char *>(embeddedTex->pcData),
        embeddedTex->mWidth, &width, &height, &nrChannels, 4);
    nrChannels = 4;
    stbiOwned = true;
  } else {
    // Raw uncompressed ARGB8888 pixel data
    width = embeddedTex->mWidth;
    height = embeddedTex->mHeight;
    nrChannels = 4;
    data = reinterpret_cast<unsigned char *>(embeddedTex->pcData);
  }

  if (!data) {
    std::cout << "Failed to decode embedded texture" << std::endl;
    return 0;
  }

  if (width <= 0 || height <= 0 || width > 16384 || height > 16384) {
    std::cout << "ERROR: Embedded texture has invalid dimensions: "
              << width << "x" << height << std::endl;
    if (stbiOwned) stbi_image_free(data);
    return 0;
  }

  std::cout << "Uploading embedded texture (" << width << "x" << height
            << ", " << nrChannels << " channels)" << std::endl;

  unsigned int textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_2D, textureID);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);

  if (stbiOwned) {
    stbi_image_free(data);
  }

  glBindTexture(GL_TEXTURE_2D, 0);

  std::cout << "Loaded embedded texture (" << width << "x" << height << ")" << std::endl;
  return textureID;
}

// Helper function to load texture from file
unsigned int loadTextureFromFile(const char *path) {
  int width, height, nrChannels;
  unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);

  if (!data) {
    std::cout << "Failed to load texture: " << path << std::endl;
    return 0;
  }

  unsigned int textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_2D, textureID);

  // Set texture parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // Upload texture data
  GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
  glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
               GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);

  stbi_image_free(data);
  glBindTexture(GL_TEXTURE_2D, 0);

  std::cout << "Loaded texture: " << path << " (" << width << "x" << height
            << ")" << std::endl;
  return textureID;
}

void Model::processNode(aiNode *node, const aiScene *scene, std::string dir) {
  // Accumulate the node's tranform
  aiMatrix4x4 nodeTransform = node->mTransformation;

  // Process each node from scene (i.e. node->meshes and children nodes)
  for (unsigned int i = 0; i < node->mNumMeshes; i++) {
    aiMesh *assimpMesh = scene->mMeshes[node->mMeshes[i]];
    Mesh mesh = processMesh(assimpMesh, scene, dir, nodeTransform);
    meshes.push_back(mesh);
  }

  // If there are children nodes, TREE TRAVERSAL
  if (node->mNumChildren > 0) {
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
      // Multiply child's tranform by parent's transform to accumulate
      node->mChildren[i]->mTransformation =
          nodeTransform * node->mChildren[i]->mTransformation;
      processNode(node->mChildren[i], scene, dir);
    }
  }

  std::cout << "Successfully traversed assimp node tree" << std::endl;
}

Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene, std::string dir,
                        const aiMatrix4x4 &transform) {
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  std::vector<Texture> textures;

  // Pre-allocate memory for performance
  vertices.reserve(mesh->mNumVertices);
  indices.reserve(mesh->mNumFaces * 3);

  for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
    Vertex vertex;

    // Positions
    aiVector3D pos = mesh->mVertices[i];
    pos = transform * pos;
    vertex.Position = glm::vec3(pos.x, pos.y, pos.z);

    // Normals
    if (mesh->HasNormals()) {
      aiVector3D norm = mesh->mNormals[i];
      norm = transform * norm;
      vertex.Normal = glm::vec3(norm.x, norm.y, norm.z);
    }

    // Texture coordinates (assimpe supports up to 8, we usually just want index
    // 0)
    if (mesh->mTextureCoords[0]) {
      vertex.TexCoords =
          glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
    } else {
      vertex.TexCoords = glm::vec2(0.0f, 0.0f);
    }
    vertices.push_back(vertex);
  }

  // Process Indices
  for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
    aiFace face = mesh->mFaces[i];
    for (unsigned int j = 0; j < face.mNumIndices; j++) {
      indices.push_back(face.mIndices[j]);
    }
  }

  // Extract textures from material
  std::cout << "Mesh material index: " << mesh->mMaterialIndex << std::endl;
  if (mesh->mMaterialIndex >= 0) {
    aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
    std::cout << "Material found: " << material << std::endl;
    if (material) {
      // Try all texture types
      std::cout << "Checking all texture types..." << std::endl;
      for (int texType = 0; texType < aiTextureType_UNKNOWN; texType++) {
        aiString texPath;
        if (material->GetTexture((aiTextureType)texType, 0, &texPath) ==
            AI_SUCCESS) {
          std::cout << "Found texture type " << texType << ": "
                    << texPath.C_Str() << std::endl;
        }
      }

      // Extract base color from material
      aiColor3D diffuseColor(0.0f, 0.0f, 0.0f);
      if (material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor) == AI_SUCCESS) {
        std::cout << "Base color (RGB): " << (float)diffuseColor.r << ", "
                  << (float)diffuseColor.g << ", " << (float)diffuseColor.b
                  << std::endl;
      }

      // Extract color for mesh
      MeshColor meshColor;
      meshColor.baseColor =
          glm::vec3(diffuseColor.r, diffuseColor.g, diffuseColor.b);

      // Load textures from customTextures folder if provided (e.g. FBX with external textures)
      if (!customTextures.baseColor.empty()) {
        std::string texFolder =
            texturesFolder.empty() ? (dir + "/textures") : texturesFolder;
        std::string baseColorPath = texFolder + "/" + customTextures.baseColor;
        unsigned int textureID = loadTextureFromFile(baseColorPath.c_str());
        if (textureID != 0) {
          Texture tex;
          tex.id = textureID;
          tex.type = "texture_diffuse";
          tex.path = baseColorPath;
          textures.push_back(tex);
        }
      }

      // Load embedded textures (e.g. GLB files with packed textures)
      if (textures.empty()) {
        aiString texPath;
        if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS) {
          const aiTexture *embeddedTex = scene->GetEmbeddedTexture(texPath.C_Str());
          if (embeddedTex) {
            unsigned int textureID = loadTextureFromMemory(embeddedTex);
            if (textureID != 0) {
              Texture tex;
              tex.id = textureID;
              tex.type = "texture_diffuse";
              tex.path = texPath.C_Str();
              textures.push_back(tex);
            }
          }
        }
      }

      return Mesh(vertices, indices, textures, meshColor);
    }
  }

  std::cout << "Mesh has " << vertices.size() << " vertices" << std::endl;
  std::cout << "First vertex position: " << vertices[0].Position.x << ", "
            << vertices[0].Position.y << ", " << vertices[0].Position.z
            << std::endl;

  // Print bounding box
  glm::vec3 minPos = vertices[0].Position;
  glm::vec3 maxPos = vertices[0].Position;
  for (const auto &v : vertices) {
    minPos = glm::min(minPos, v.Position);
    maxPos = glm::max(maxPos, v.Position);
  }
  std::cout << "Min: " << minPos.x << ", " << minPos.y << ", " << minPos.z
            << std::endl;
  std::cout << "Max: " << maxPos.x << ", " << maxPos.y << ", " << maxPos.z
            << std::endl;

  return Mesh(vertices, indices, textures, MeshColor{{1.0f, 1.0f, 1.0f}});
}
