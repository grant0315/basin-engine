#ifndef MODEL_H
#define MODEL_H

#include "mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/matrix4x4.h>
#include <assimp/postprocess.h>
#include <filesystem>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

struct ModelTextures {
  std::string baseColor;
  std::string normal;
  std::string height;
  std::string roughness;
};

class Model {
public:
  std::vector<Mesh> meshes;
  std::string directory;
  std::string texturesFolder;
  ModelTextures customTextures;

  Model(std::string const &path, std::string const &texturesDir = "", ModelTextures textures = {}) {
    texturesFolder = texturesDir;
    customTextures = textures;
    loadModel(path);
  }
  Model(std::vector<Vertex> vertices, std::vector<unsigned int> indices,
        std::vector<Texture> textures = std::vector<Texture>()) {
    meshes.push_back(Mesh(vertices, indices, textures));
  };

  // --- Getters ---
  glm::vec3 getModelCenter() {
    if (!m_calculatedBoundingBox) {
      calculateBoundingBox();
    }
    return (m_minPos + m_maxPos) / 2.0f;
  }

  std::vector<glm::vec3> getBoundingBox() {
    if (!m_calculatedBoundingBox) {
      calculateBoundingBox();
    }
    return {m_minPos, m_maxPos};
  }

  void render() {
    for (unsigned int i = 0; i < meshes.size(); i++) {
      meshes[i].Render();
    }
  }

private:
  bool m_calculatedBoundingBox = false;
  glm::vec3 m_minPos;
  glm::vec3 m_maxPos;

  // Texture cache: maps texture path/reference to GPU texture ID
  std::unordered_map<std::string, unsigned int> m_textureCache;

  void calculateBoundingBox() {
    if (meshes.empty())
      return;

    // Start with the first mesh's bounding box
    AABB firstAABB = meshes[0].getAxisAlignedBoundingBox();
    m_minPos = firstAABB.center - glm::vec3(firstAABB.xHalfExtent, firstAABB.yHalfExtent, firstAABB.zHalfExtent);
    m_maxPos = firstAABB.center + glm::vec3(firstAABB.xHalfExtent, firstAABB.yHalfExtent, firstAABB.zHalfExtent);

    for (unsigned int i = 1; i < meshes.size(); i++) {
      AABB meshAABB = meshes[i].getAxisAlignedBoundingBox();
      glm::vec3 meshMin = meshAABB.center - glm::vec3(meshAABB.xHalfExtent, meshAABB.yHalfExtent, meshAABB.zHalfExtent);
      glm::vec3 meshMax = meshAABB.center + glm::vec3(meshAABB.xHalfExtent, meshAABB.yHalfExtent, meshAABB.zHalfExtent);
      m_minPos = glm::min(m_minPos, meshMin);
      m_maxPos = glm::max(m_maxPos, meshMax);
    }
    m_calculatedBoundingBox = true;
  }
  void loadModel(std::string const &path) {
    Assimp::Importer importer;
    const aiScene *scene =
        importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs |
                                     aiProcess_CalcTangentSpace);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
        !scene->mRootNode) {
      // TODO: Handle errors here
      std::cout << "ERROR::ASSIMP::LOAD_FAILED path: " << path << std::endl;
      return;
    }
    directory = std::filesystem::path(path).parent_path().string();

    // Start recurisve processing from the root
    processNode(scene->mRootNode, scene, directory);
  }

  void processNode(aiNode *node, const aiScene *scene, std::string dir);
  Mesh processMesh(aiMesh *mesh, const aiScene *scene, std::string dir,
                   const aiMatrix4x4 &transform = aiMatrix4x4());
};

#endif
