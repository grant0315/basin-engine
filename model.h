#include "mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <iostream>

class Model {
public:
  std::vector<Mesh> meshes;
  std::string directory;

  Model(std::string const &path) { loadModel(path); }

private:
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
    processNode(scene->mRootNode, scene);
  }

  void processNode(aiNode *node, const aiScene *scene);
  Mesh processMesh(aiMesh *mesh, const aiScene *scene);
};
