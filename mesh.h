#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <glm/gtc/matrix_transform.hpp>
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

class Mesh {
public:
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  std::vector<Texture> textures;

  Mesh(std::vector<Vertex> v, std::vector<unsigned int> i,
       std::vector<Texture> t);

  void Render();

  // --- GETTERS ----
  unsigned int GetVAO() { return VAO; }
  unsigned int GetVBO() { return VBO; }
  unsigned int GetEBO() { return EBO; }

private:
  unsigned int VAO, VBO, EBO;
  void setupMesh();
};
