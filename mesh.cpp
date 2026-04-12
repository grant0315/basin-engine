#include "glad/glad.h"

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

  setupMesh();
}

void Mesh::setupMesh() {
  // generate buffers
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &VAO);
  glGenBuffers(1, &EBO);

  // Bind VAO, VBO
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBindVertexArray(VAO);

  // Set data for VAO, VBO
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
               vertices.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
  glEnableVertexAttribArray(0);

  // Bind and upload indicies to EBO
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
               indices.data(), GL_STATIC_DRAW);
}

void Mesh::Render() {
  glBindVertexArray(VAO);
  glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, (void *)0);
}
