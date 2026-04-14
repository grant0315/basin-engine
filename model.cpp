#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <iostream>
#include <vector>

#include "model.h"

void Model::processNode(aiNode *node, const aiScene *scene) {
  // Process each node from scene (i.e. node->meshes and children nodes)
  for (unsigned int i = 0; i < node->mNumMeshes; i++) {
    aiMesh *assimpMesh = scene->mMeshes[node->mMeshes[i]];
    Mesh mesh = processMesh(assimpMesh, scene);
    meshes.push_back(mesh);
  }

  // If there are children nodes, TREE TRAVERSAL
  if (node->mNumChildren > 0) {
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
      processNode(node->mChildren[i], scene);
    }
  }

  std::cout << "Successfully traversed assimp node tree" << std::endl;
}

Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene) {
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

  return Mesh(vertices, indices, textures);
}
