#ifndef MODEL_H
#define MODEL_H

#include "mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
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

  void calculateBoundingBox() {
    if (meshes.empty())
      return;

    // Start with the first mesh's bounding box
    std::vector<glm::vec3> firstBox = meshes[0].getBoundingBox();
    m_minPos = firstBox[0];
    m_maxPos = firstBox[1];

    for (unsigned int i = 1; i < meshes.size(); i++) {
      std::vector<glm::vec3> box = meshes[i].getBoundingBox();
      m_minPos = glm::min(m_minPos, box[0]);
      m_maxPos = glm::max(m_maxPos, box[1]);
    }
    m_calculatedBoundingBox = true;
  }
  void loadModel(std::string const &path) {
    Assimp::Importer importer;
    const aiScene *scene =
        importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
        !scene->mRootNode) {
      // TODO: Handle errors here
      std::cout << "ERROR::ASSIMP::LOAD_FAILED path: " << path << std::endl;
      return;
    }
    directory = path.substr(0, path.find_last_of('/'));

    // Start recurisve processing from the root
    processNode(scene->mRootNode, scene, directory);
  }

  void processNode(aiNode *node, const aiScene *scene, std::string dir);
  Mesh processMesh(aiMesh *mesh, const aiScene *scene, std::string dir);
};

#endif
