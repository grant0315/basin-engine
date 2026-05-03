#include "glad/glad.h"

#include "basin/renderer/mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

Mesh::Mesh(std::vector<Vertex> v, std::vector<unsigned int> i,
           std::vector<Texture> t, MeshColor c) {
  vertices = v;
  indices = i;
  textures = t;
  color = c;

  setupMesh();
}

void Mesh::setupMesh() {
  // generate buffers
  glGenBuffers(1, &VBO);
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &EBO);

  // Bind VAO, VBO
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);

  // Set data for VAO, VBO
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
               vertices.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)(3 * sizeof(float)));
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)(6 * sizeof(float)));
  // Tangent
  glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, Tangent));
  // Bitangent
  glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, Bitangent));
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);
  glEnableVertexAttribArray(3);
  glEnableVertexAttribArray(4);

  // Bind and upload indicies to EBO
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
               indices.data(), GL_STATIC_DRAW);
}

glm::vec3 Mesh::getMeshCenter() {
  return getAxisAlignedBoundingBox().center;
}

AABB Mesh::getAxisAlignedBoundingBox() {
  if (m_aabbCached)
    return m_cachedAABB;

  glm::vec3 minPos = vertices[0].Position;
  glm::vec3 maxPos = vertices[0].Position;
  for (const auto &v : vertices) {
    minPos = glm::min(minPos, v.Position);
    maxPos = glm::max(maxPos, v.Position);
  }

  AABB aabb;
  aabb.center = (minPos + maxPos) / 2.0f;
  aabb.xHalfExtent = (maxPos.x - minPos.x) / 2.0f;
  aabb.yHalfExtent = (maxPos.y - minPos.y) / 2.0f;
  aabb.zHalfExtent = (maxPos.z - minPos.z) / 2.0f;
  m_cachedAABB = aabb;
  m_aabbCached = true;
  return m_cachedAABB;
}

void Mesh::Render() {
  glBindVertexArray(VAO);
  glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, (void *)0);
}
