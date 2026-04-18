#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>
#include <vector>

#include "model.h"

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
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  
  // Upload texture data
  GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
  glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);
  
  stbi_image_free(data);
  glBindTexture(GL_TEXTURE_2D, 0);
  
  std::cout << "Loaded texture: " << path << " (" << width << "x" << height << ")" << std::endl;
  return textureID;
}

void Model::processNode(aiNode *node, const aiScene *scene, std::string dir) {
  // Process each node from scene (i.e. node->meshes and children nodes)
  for (unsigned int i = 0; i < node->mNumMeshes; i++) {
    aiMesh *assimpMesh = scene->mMeshes[node->mMeshes[i]];
    Mesh mesh = processMesh(assimpMesh, scene, dir);
    meshes.push_back(mesh);
  }

  // If there are children nodes, TREE TRAVERSAL
  if (node->mNumChildren > 0) {
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
      processNode(node->mChildren[i], scene, dir);
    }
  }

  std::cout << "Successfully traversed assimp node tree" << std::endl;
}

Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene, std::string dir) {
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  std::vector<Texture> textures;

  // Pre-allocate memory for performance
  vertices.reserve(mesh->mNumVertices);
  indices.reserve(mesh->mNumFaces * 3);

  for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
    Vertex vertex;

    // Positions
    vertex.Position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y,
                                mesh->mVertices[i].z);

    // Normals
    if (mesh->HasNormals()) {
      vertex.Normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y,
                                mesh->mNormals[i].y);
    }

    // Texture coordinates (assimpe supports up to 8, we usually just want index 0)
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
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    std::cout << "Material found: " << material << std::endl;
    if (material) {
      // Try all texture types
      std::cout << "Checking all texture types..." << std::endl;
      for (int texType = 0; texType < aiTextureType_UNKNOWN; texType++) {
        aiString texPath;
        if (material->GetTexture((aiTextureType)texType, 0, &texPath) == AI_SUCCESS) {
          std::cout << "Found texture type " << texType << ": " << texPath.C_Str() << std::endl;
        }
      }
      
      // Extract base color from material
      aiColor3D diffuseColor(0.0f, 0.0f, 0.0f);
      if (material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor) == AI_SUCCESS) {
        std::cout << "Base color (RGB): " << (float)diffuseColor.r << ", " 
                  << (float)diffuseColor.g << ", " << (float)diffuseColor.b << std::endl;
      }

      // Extract color for mesh
      MeshColor meshColor;
      meshColor.baseColor = glm::vec3(diffuseColor.r, diffuseColor.g, diffuseColor.b);
      
      // Try to load texture from textures folder
      std::string texturePath = dir + "/../textures/shop_1_shop_1_BaseColor.png";
      unsigned int textureID = loadTextureFromFile(texturePath.c_str());
      if (textureID != 0) {
        Texture tex;
        tex.id = textureID;
        tex.type = "texture_diffuse";
        tex.path = texturePath;
        textures.push_back(tex);
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
