#include "basin/scene/primitive_generator.h"
#include "basin/renderer/model.h"
#include <cmath>

static void computeTangents(std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices) {
  for (size_t i = 0; i < indices.size(); i += 3) {
    unsigned int i0 = indices[i];
    unsigned int i1 = indices[i + 1];
    unsigned int i2 = indices[i + 2];

    glm::vec3 edge1 = vertices[i1].Position - vertices[i0].Position;
    glm::vec3 edge2 = vertices[i2].Position - vertices[i0].Position;
    glm::vec2 deltaUV1 = vertices[i1].TexCoords - vertices[i0].TexCoords;
    glm::vec2 deltaUV2 = vertices[i2].TexCoords - vertices[i0].TexCoords;

    float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
    glm::vec3 tangent;
    if (std::abs(f) < 1e-6f) {
      tangent = glm::vec3(1, 0, 0);
    } else {
      tangent = f * (deltaUV2.y * edge1 - deltaUV1.y * edge2);
    }
    glm::vec3 bitangent;
    if (std::abs(f) < 1e-6f) {
      bitangent = glm::cross(vertices[i0].Normal, tangent);
    } else {
      bitangent = f * (-deltaUV2.x * edge1 + deltaUV1.x * edge2);
    }

    for (int j = 0; j < 3; j++) {
      unsigned int idx = indices[i + j];
      vertices[idx].Tangent = tangent;
      vertices[idx].Bitangent = bitangent;
    }
  }
}

Model *PrimitiveGenerator::generateCube(float size) {
  float halfSize = size / 2.0f;
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;

  // Top face (y = +halfSize, normal points up)
  vertices.push_back({glm::vec3(halfSize, halfSize, halfSize),
                      glm::vec3(0, 1, 0), glm::vec2(1, 1),
                      glm::vec3(0), glm::vec3(0)}); // 0
  vertices.push_back({glm::vec3(-halfSize, halfSize, halfSize),
                      glm::vec3(0, 1, 0), glm::vec2(0, 1),
                      glm::vec3(0), glm::vec3(0)}); // 1
  vertices.push_back({glm::vec3(halfSize, halfSize, -halfSize),
                      glm::vec3(0, 1, 0), glm::vec2(1, 0),
                      glm::vec3(0), glm::vec3(0)}); // 2
  vertices.push_back({glm::vec3(-halfSize, halfSize, -halfSize),
                      glm::vec3(0, 1, 0), glm::vec2(0, 0),
                      glm::vec3(0), glm::vec3(0)}); // 3

  // Bottom face (y = -halfSize, normal points down)
  vertices.push_back({glm::vec3(halfSize, -halfSize, halfSize),
                      glm::vec3(0, -1, 0), glm::vec2(1, 1),
                      glm::vec3(0), glm::vec3(0)}); // 4
  vertices.push_back({glm::vec3(-halfSize, -halfSize, halfSize),
                      glm::vec3(0, -1, 0), glm::vec2(0, 1),
                      glm::vec3(0), glm::vec3(0)}); // 5
  vertices.push_back({glm::vec3(halfSize, -halfSize, -halfSize),
                      glm::vec3(0, -1, 0), glm::vec2(1, 0),
                      glm::vec3(0), glm::vec3(0)}); // 6
  vertices.push_back({glm::vec3(-halfSize, -halfSize, -halfSize),
                      glm::vec3(0, -1, 0), glm::vec2(0, 0),
                      glm::vec3(0), glm::vec3(0)}); // 7

  // Front face (z = +halfSize, normal points forward)
  vertices.push_back({glm::vec3(halfSize, halfSize, halfSize),
                      glm::vec3(0, 0, 1), glm::vec2(1, 1),
                      glm::vec3(0), glm::vec3(0)}); // 8
  vertices.push_back({glm::vec3(-halfSize, halfSize, halfSize),
                      glm::vec3(0, 0, 1), glm::vec2(0, 1),
                      glm::vec3(0), glm::vec3(0)}); // 9
  vertices.push_back({glm::vec3(halfSize, -halfSize, halfSize),
                      glm::vec3(0, 0, 1), glm::vec2(1, 0),
                      glm::vec3(0), glm::vec3(0)}); // 10
  vertices.push_back({glm::vec3(-halfSize, -halfSize, halfSize),
                      glm::vec3(0, 0, 1), glm::vec2(0, 0),
                      glm::vec3(0), glm::vec3(0)}); // 11

  // Back face (z = -halfSize, normal points backward)
  vertices.push_back({glm::vec3(halfSize, halfSize, -halfSize),
                      glm::vec3(0, 0, -1), glm::vec2(0, 1),
                      glm::vec3(0), glm::vec3(0)}); // 12
  vertices.push_back({glm::vec3(-halfSize, halfSize, -halfSize),
                      glm::vec3(0, 0, -1), glm::vec2(1, 1),
                      glm::vec3(0), glm::vec3(0)}); // 13
  vertices.push_back({glm::vec3(halfSize, -halfSize, -halfSize),
                      glm::vec3(0, 0, -1), glm::vec2(0, 0),
                      glm::vec3(0), glm::vec3(0)}); // 14
  vertices.push_back({glm::vec3(-halfSize, -halfSize, -halfSize),
                      glm::vec3(0, 0, -1), glm::vec2(1, 0),
                      glm::vec3(0), glm::vec3(0)}); // 15

  // Right face (x = +halfSize, normal points right)
  vertices.push_back({glm::vec3(halfSize, halfSize, halfSize),
                      glm::vec3(1, 0, 0), glm::vec2(0, 1),
                      glm::vec3(0), glm::vec3(0)}); // 16
  vertices.push_back({glm::vec3(halfSize, halfSize, -halfSize),
                      glm::vec3(1, 0, 0), glm::vec2(1, 1),
                      glm::vec3(0), glm::vec3(0)}); // 17
  vertices.push_back({glm::vec3(halfSize, -halfSize, halfSize),
                      glm::vec3(1, 0, 0), glm::vec2(0, 0),
                      glm::vec3(0), glm::vec3(0)}); // 18
  vertices.push_back({glm::vec3(halfSize, -halfSize, -halfSize),
                      glm::vec3(1, 0, 0), glm::vec2(1, 0),
                      glm::vec3(0), glm::vec3(0)}); // 19

  // Left face (x = -halfSize, normal points left)
  vertices.push_back({glm::vec3(-halfSize, halfSize, halfSize),
                      glm::vec3(-1, 0, 0), glm::vec2(1, 1),
                      glm::vec3(0), glm::vec3(0)}); // 20
  vertices.push_back({glm::vec3(-halfSize, halfSize, -halfSize),
                      glm::vec3(-1, 0, 0), glm::vec2(0, 1),
                      glm::vec3(0), glm::vec3(0)}); // 21
  vertices.push_back({glm::vec3(-halfSize, -halfSize, halfSize),
                      glm::vec3(-1, 0, 0), glm::vec2(1, 0),
                      glm::vec3(0), glm::vec3(0)}); // 22
  vertices.push_back({glm::vec3(-halfSize, -halfSize, -halfSize),
                      glm::vec3(-1, 0, 0), glm::vec2(0, 0),
                      glm::vec3(0), glm::vec3(0)}); // 23

  // Indices (counter-clockwise winding when viewed from outside)
  // Top face
  indices.insert(indices.end(), {0, 2, 1, 2, 3, 1});
  // Bottom face
  indices.insert(indices.end(), {4, 5, 6, 5, 7, 6});
  // Front face
  indices.insert(indices.end(), {8, 10, 9, 10, 11, 9});
  // Back face
  indices.insert(indices.end(), {12, 13, 14, 13, 15, 14});
  // Right face
  indices.insert(indices.end(), {16, 18, 17, 18, 19, 17});
  // Left face
  indices.insert(indices.end(), {20, 21, 22, 21, 23, 22});

  computeTangents(vertices, indices);
  return new Model(vertices, indices);
}

Model *PrimitiveGenerator::generateCuboid(float length, float width,
                                           float height) {
  float halfLength = length / 2.0f;
  float halfWidth = width / 2.0f;
  float halfHeight = height / 2.0f;
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;

  // Top face (y = +halfHeight, normal points up)
  vertices.push_back({glm::vec3(halfLength, halfHeight, halfWidth),
                      glm::vec3(0, 1, 0), glm::vec2(1, 1),
                      glm::vec3(0), glm::vec3(0)}); // 0
  vertices.push_back({glm::vec3(-halfLength, halfHeight, halfWidth),
                      glm::vec3(0, 1, 0), glm::vec2(0, 1),
                      glm::vec3(0), glm::vec3(0)}); // 1
  vertices.push_back({glm::vec3(halfLength, halfHeight, -halfWidth),
                      glm::vec3(0, 1, 0), glm::vec2(1, 0),
                      glm::vec3(0), glm::vec3(0)}); // 2
  vertices.push_back({glm::vec3(-halfLength, halfHeight, -halfWidth),
                      glm::vec3(0, 1, 0), glm::vec2(0, 0),
                      glm::vec3(0), glm::vec3(0)}); // 3

  // Bottom face (y = -halfHeight, normal points down)
  vertices.push_back({glm::vec3(halfLength, -halfHeight, halfWidth),
                      glm::vec3(0, -1, 0), glm::vec2(1, 1),
                      glm::vec3(0), glm::vec3(0)}); // 4
  vertices.push_back({glm::vec3(-halfLength, -halfHeight, halfWidth),
                      glm::vec3(0, -1, 0), glm::vec2(0, 1),
                      glm::vec3(0), glm::vec3(0)}); // 5
  vertices.push_back({glm::vec3(halfLength, -halfHeight, -halfWidth),
                      glm::vec3(0, -1, 0), glm::vec2(1, 0),
                      glm::vec3(0), glm::vec3(0)}); // 6
  vertices.push_back({glm::vec3(-halfLength, -halfHeight, -halfWidth),
                      glm::vec3(0, -1, 0), glm::vec2(0, 0),
                      glm::vec3(0), glm::vec3(0)}); // 7

  // Front face (z = +halfWidth, normal points forward)
  vertices.push_back({glm::vec3(halfLength, halfHeight, halfWidth),
                      glm::vec3(0, 0, 1), glm::vec2(1, 1),
                      glm::vec3(0), glm::vec3(0)}); // 8
  vertices.push_back({glm::vec3(-halfLength, halfHeight, halfWidth),
                      glm::vec3(0, 0, 1), glm::vec2(0, 1),
                      glm::vec3(0), glm::vec3(0)}); // 9
  vertices.push_back({glm::vec3(halfLength, -halfHeight, halfWidth),
                      glm::vec3(0, 0, 1), glm::vec2(1, 0),
                      glm::vec3(0), glm::vec3(0)}); // 10
  vertices.push_back({glm::vec3(-halfLength, -halfHeight, halfWidth),
                      glm::vec3(0, 0, 1), glm::vec2(0, 0),
                      glm::vec3(0), glm::vec3(0)}); // 11

  // Back face (z = -halfWidth, normal points backward)
  vertices.push_back({glm::vec3(halfLength, halfHeight, -halfWidth),
                      glm::vec3(0, 0, -1), glm::vec2(0, 1),
                      glm::vec3(0), glm::vec3(0)}); // 12
  vertices.push_back({glm::vec3(-halfLength, halfHeight, -halfWidth),
                      glm::vec3(0, 0, -1), glm::vec2(1, 1),
                      glm::vec3(0), glm::vec3(0)}); // 13
  vertices.push_back({glm::vec3(halfLength, -halfHeight, -halfWidth),
                      glm::vec3(0, 0, -1), glm::vec2(0, 0),
                      glm::vec3(0), glm::vec3(0)}); // 14
  vertices.push_back({glm::vec3(-halfLength, -halfHeight, -halfWidth),
                      glm::vec3(0, 0, -1), glm::vec2(1, 0),
                      glm::vec3(0), glm::vec3(0)}); // 15

  // Right face (x = +halfLength, normal points right)
  vertices.push_back({glm::vec3(halfLength, halfHeight, halfWidth),
                      glm::vec3(1, 0, 0), glm::vec2(0, 1),
                      glm::vec3(0), glm::vec3(0)}); // 16
  vertices.push_back({glm::vec3(halfLength, halfHeight, -halfWidth),
                      glm::vec3(1, 0, 0), glm::vec2(1, 1),
                      glm::vec3(0), glm::vec3(0)}); // 17
  vertices.push_back({glm::vec3(halfLength, -halfHeight, halfWidth),
                      glm::vec3(1, 0, 0), glm::vec2(0, 0),
                      glm::vec3(0), glm::vec3(0)}); // 18
  vertices.push_back({glm::vec3(halfLength, -halfHeight, -halfWidth),
                      glm::vec3(1, 0, 0), glm::vec2(1, 0),
                      glm::vec3(0), glm::vec3(0)}); // 19

  // Left face (x = -halfLength, normal points left)
  vertices.push_back({glm::vec3(-halfLength, halfHeight, halfWidth),
                      glm::vec3(-1, 0, 0), glm::vec2(1, 1),
                      glm::vec3(0), glm::vec3(0)}); // 20
  vertices.push_back({glm::vec3(-halfLength, halfHeight, -halfWidth),
                      glm::vec3(-1, 0, 0), glm::vec2(0, 1),
                      glm::vec3(0), glm::vec3(0)}); // 21
  vertices.push_back({glm::vec3(-halfLength, -halfHeight, halfWidth),
                      glm::vec3(-1, 0, 0), glm::vec2(1, 0),
                      glm::vec3(0), glm::vec3(0)}); // 22
  vertices.push_back({glm::vec3(-halfLength, -halfHeight, -halfWidth),
                      glm::vec3(-1, 0, 0), glm::vec2(0, 0),
                      glm::vec3(0), glm::vec3(0)}); // 23

  // Indices (counter-clockwise winding when viewed from outside)
  // Top face
  indices.insert(indices.end(), {0, 2, 1, 2, 3, 1});
  // Bottom face
  indices.insert(indices.end(), {4, 5, 6, 5, 7, 6});
  // Front face
  indices.insert(indices.end(), {8, 10, 9, 10, 11, 9});
  // Back face
  indices.insert(indices.end(), {12, 13, 14, 13, 15, 14});
  // Right face
  indices.insert(indices.end(), {16, 18, 17, 18, 19, 17});
  // Left face
  indices.insert(indices.end(), {20, 21, 22, 21, 23, 22});

  computeTangents(vertices, indices);
  return new Model(vertices, indices);
}

Model *PrimitiveGenerator::generatePlane(float length, float depth, float thickness) {
  float halfLength = length / 2.0f;
  float halfDepth = depth / 2.0f;
  float halfThickness = thickness / 2.0f;
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;

  // Create a thin box/slab with thickness in Y
  // Top face (y = +halfThickness, normal points up)
  vertices.push_back({glm::vec3(halfLength, halfThickness, halfDepth),
                      glm::vec3(0, 1, 0), glm::vec2(1, 1),
                      glm::vec3(0), glm::vec3(0)});
  vertices.push_back({glm::vec3(-halfLength, halfThickness, halfDepth),
                      glm::vec3(0, 1, 0), glm::vec2(0, 1),
                      glm::vec3(0), glm::vec3(0)});
  vertices.push_back({glm::vec3(halfLength, halfThickness, -halfDepth),
                      glm::vec3(0, 1, 0), glm::vec2(1, 0),
                      glm::vec3(0), glm::vec3(0)});
  vertices.push_back({glm::vec3(-halfLength, halfThickness, -halfDepth),
                      glm::vec3(0, 1, 0), glm::vec2(0, 0),
                      glm::vec3(0), glm::vec3(0)});

  // Bottom face (y = -halfThickness, normal points down)
  vertices.push_back({glm::vec3(halfLength, -halfThickness, halfDepth),
                      glm::vec3(0, -1, 0), glm::vec2(1, 1),
                      glm::vec3(0), glm::vec3(0)});
  vertices.push_back({glm::vec3(-halfLength, -halfThickness, halfDepth),
                      glm::vec3(0, -1, 0), glm::vec2(0, 1),
                      glm::vec3(0), glm::vec3(0)});
  vertices.push_back({glm::vec3(halfLength, -halfThickness, -halfDepth),
                      glm::vec3(0, -1, 0), glm::vec2(1, 0),
                      glm::vec3(0), glm::vec3(0)});
  vertices.push_back({glm::vec3(-halfLength, -halfThickness, -halfDepth),
                      glm::vec3(0, -1, 0), glm::vec2(0, 0),
                      glm::vec3(0), glm::vec3(0)});

  // Indices for top and bottom faces
  indices.insert(indices.end(), {0, 2, 1, 2, 3, 1}); // Top
  indices.insert(indices.end(), {4, 5, 6, 5, 7, 6}); // Bottom

  computeTangents(vertices, indices);
  return new Model(vertices, indices);
}
