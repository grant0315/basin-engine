#ifndef MESH_H
#define MESH_H

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <vector>

struct Vertex {
  glm::vec3 Position;
  glm::vec3 Normal;
  glm::vec2 TexCoords;
};

struct Texture {
  unsigned int id;
  std::string type; // e.g. "texture_diffuse"
  std::string path;
};

struct MeshColor {
  glm::vec3 baseColor;
};

class Mesh {
public:
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  std::vector<Texture> textures;
  MeshColor color;

  Mesh(std::vector<Vertex> v, std::vector<unsigned int> i,
       std::vector<Texture> t, MeshColor c = MeshColor{{1.0f, 1.0f, 1.0f}});

  std::vector<glm::vec3> getBoundingBox();
  glm::vec3 getMeshCenter();

  void Render();

  // --- GETTERS ----
  unsigned int GetVAO() { return VAO; }
  unsigned int GetVBO() { return VBO; }
  unsigned int GetEBO() { return EBO; }
  glm::vec3 GetColor() { return color.baseColor; }

private:
  unsigned int VAO, VBO, EBO;
  void setupMesh();
};

#endif
