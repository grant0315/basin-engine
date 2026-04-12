#include "mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

Mesh::Mesh(std::vector<Vertex> v, std::vector<unsigned int> i,
           std::vector<Texture> t) {
  vertices = v;
  indices = i;
  textures = t;
}
