#ifndef ENTITY_H
#define ENTITY_H

#include "basin/renderer/model.h"
#include "basin/renderer/shader.h"
#include <glad/glad.h>
#include <glm/common.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <iostream>
#include <string>

class Entity {
public:
  Entity(const std::string &entName, Model *model, bool isCollidable) {
    m_name = entName;
    m_model = model;
    m_isCollidable = isCollidable;

    // Set defaults for Model position
    m_position = glm::vec3(1.0f, 1.0f, 1.0f);
    m_rotation = glm::quat();
    m_scale = glm::vec3(1.0f, 1.0f, 1.0f);

    m_dirty = true; // set flag to ensure model calculation after init
    m_dirtyAABB = true;

    std::cout << "Initialized Entity: " << m_name << std::endl;
  }

  Entity(const std::string &entName, Model *model, glm::vec3 pos, glm::quat rot,
         glm::vec3 scale, bool isCollidable) {
    m_name = entName;
    m_model = model;
    m_isCollidable = isCollidable;

    // Set defaults for model pos, rot, and scale
    m_position = pos;
    m_rotation = rot;
    m_scale = scale;

    m_dirty = true; // set flag to ensure model calculation after init
    m_dirtyAABB = true;

    std::cout << "Initialized Entity: " << m_name << std::endl;
  }

  // --- Getters ---
  glm::mat4 getModelMatrix() {
    if (m_dirty) {
      glm::mat4 translation = glm::translate(glm::mat4(1.0f), m_position);
      glm::mat4 rotation = glm::mat4_cast(m_rotation);
      glm::mat4 scale = glm::scale(glm::mat4(1.0f), m_scale);

      m_modelMatrix = translation * rotation * scale;
    }
    return m_modelMatrix;
  }

  glm::vec3 getWorldCenter() {
    glm::vec3 modelCenter = m_model->getModelCenter();
    glm::vec4 center4 = glm::vec4(modelCenter, 1.0f);
    glm::vec4 worldCenter4 = getModelMatrix() * center4;
    return glm::vec3(worldCenter4);
  }

  AABB getAxisAlignedBoundingBox() {
    std::vector<glm::vec3> boundingBox = m_model->getBoundingBox();

    // Create AABB object and write to member variable
    m_AABB.center = getWorldCenter();

    // Get face extents from min corner max corner
    m_AABB.xHalfExtent =
        ((boundingBox[1].x - boundingBox[0].x) / 2) * m_scale.x;
    m_AABB.yHalfExtent =
        ((boundingBox[1].y - boundingBox[0].y) / 2) * m_scale.y;
    m_AABB.zHalfExtent =
        ((boundingBox[1].z - boundingBox[0].z) / 2) * m_scale.z;

    m_dirtyAABB = false;

    return m_AABB;
  }

  AABB getAxisAlignedBoundingBoxAtPosition(glm::vec3 pos) {
    AABB tempAABB = getAxisAlignedBoundingBox();
    tempAABB.center = pos;
    return tempAABB;
  }

  // Returns one AABB per mesh, transformed to world space (cached until dirty)
  const std::vector<AABB> &getMeshAABBs() {
    if (!m_dirtyAABB && !m_cachedMeshAABBs.empty()) {
      return m_cachedMeshAABBs;
    }

    m_cachedMeshAABBs.clear();
    glm::mat4 modelMatrix = getModelMatrix();

    for (unsigned int i = 0; i < m_model->meshes.size(); i++) {
      Mesh &mesh = m_model->meshes[i];
      AABB meshAABB = mesh.getAxisAlignedBoundingBox();

      // Transform center to world space
      glm::vec4 worldCenter4 = modelMatrix * glm::vec4(meshAABB.center, 1.0f);

      AABB aabb;
      aabb.center = glm::vec3(worldCenter4);
      aabb.xHalfExtent = meshAABB.xHalfExtent * m_scale.x;
      aabb.yHalfExtent = meshAABB.yHalfExtent * m_scale.y;
      aabb.zHalfExtent = meshAABB.zHalfExtent * m_scale.z;
      m_cachedMeshAABBs.push_back(aabb);
    }
    m_dirtyAABB = false;
    return m_cachedMeshAABBs;
  }

  // Returns per-mesh AABBs as if the entity were at a different position
  std::vector<AABB> getMeshAABBsAtPosition(glm::vec3 pos) {
    glm::vec3 offset = pos - m_position;
    const std::vector<AABB> &cached = getMeshAABBs();
    std::vector<AABB> aabbs(cached); // copy
    for (auto &aabb : aabbs) {
      aabb.center += offset;
    }
    return aabbs;
  }

  glm::vec3 getPosition() const { return m_position; }
  glm::quat getRotation() const { return m_rotation; }
  glm::vec3 getScale() const { return m_scale; }
  glm::vec3 getModelCenter() const { return m_model->getModelCenter(); }
  std::string getName() { return m_name; }
  bool isCollidable() const { return m_isCollidable; }

  // --- Setters ---
  void setPosition(glm::vec3 pos) {
    m_position = pos;
    m_dirty = true;
    m_dirtyAABB = true;
  }

  void setScale(glm::vec3 scale) {
    m_scale = scale;
    m_dirty = true;
    m_dirtyAABB = true;
  }

  void setRotationEuler(glm::vec3 eulerDegrees) {
    m_rotation = glm::quat(glm::radians(eulerDegrees));
    m_dirty = true;
  }

  void setRotationQuat(glm::quat rot) {
    m_rotation = rot;
    m_dirty = true;
  }

  void setName(std::string name) { m_name = name; }

  // Set base color on all meshes (RGB from vec4, alpha stored separately)
  void setColor(glm::vec4 color) {
    m_alpha = color.a;
    for (auto &mesh : m_model->meshes) {
      mesh.color.baseColor = glm::vec3(color.r, color.g, color.b);
    }
  }

  glm::vec4 getColor() const {
    if (m_model->meshes.empty()) return glm::vec4(1.0f);
    glm::vec3 c = m_model->meshes[0].color.baseColor;
    return glm::vec4(c.r, c.g, c.b, m_alpha);
  }

  // --- Transform Operations ---
  // Rotates the entity aroudn a specific axis (i.e. glm::vec3(0, 1, 0) for y)
  void rotate(float angleDegrees, glm::vec3 axis) {
    float rad = glm::radians(angleDegrees);
    m_rotation = glm::rotate(m_rotation, rad, axis);
    m_dirty = true;
  }

  void Draw(Shader &shader) {
    // 1. Calculate the model matrix from position, rotation, and scale
    // 2. Pass the matrix to the shader
    // 3. Render each mesh with its textures
    glm::mat4 modelMatrix = getModelMatrix();
    shader.setUniform("model", modelMatrix);
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));
    shader.setUniform("normalMatrix", normalMatrix);

    for (unsigned int i = 0; i < m_model->meshes.size(); i++) {
      Mesh &mesh = m_model->meshes[i];

      // Set base color and alpha
      shader.setUniform("objectColor", mesh.GetColor());
      shader.setUniform("objectAlpha", m_alpha);

      // Reset texture flags
      shader.setUniform("hasTexture", false);
      shader.setUniform("hasNormalMap", false);
      shader.setUniform("hasHeightMap", false);
      shader.setUniform("hasRoughnessMap", false);

      // Bind each texture to the correct unit by type
      for (const Texture &tex : mesh.textures) {
        if (tex.type == "texture_diffuse") {
          glActiveTexture(GL_TEXTURE0);
          glBindTexture(GL_TEXTURE_2D, tex.id);
          shader.setUniform("texture_diffuse", 0);
          shader.setUniform("hasTexture", true);
        } else if (tex.type == "texture_normal") {
          glActiveTexture(GL_TEXTURE1);
          glBindTexture(GL_TEXTURE_2D, tex.id);
          shader.setUniform("texture_normal", 1);
          shader.setUniform("hasNormalMap", true);
        } else if (tex.type == "texture_height") {
          glActiveTexture(GL_TEXTURE2);
          glBindTexture(GL_TEXTURE_2D, tex.id);
          shader.setUniform("texture_height", 2);
          shader.setUniform("hasHeightMap", true);
        } else if (tex.type == "texture_roughness") {
          glActiveTexture(GL_TEXTURE3);
          glBindTexture(GL_TEXTURE_2D, tex.id);
          shader.setUniform("texture_roughness", 3);
          shader.setUniform("hasRoughnessMap", true);
        }
      }

      mesh.Render();
    }

    m_dirty = false; // Reset dirty flag
  }

private:
  bool m_dirty; // Flag for needing rebuild of matrix
  bool m_dirtyAABB;
  bool m_isCollidable;

  std::string m_name;
  Model *m_model;
  glm::mat4 m_modelMatrix;

  glm::vec3 m_position;
  glm::quat m_rotation;
  float m_alpha = 1.0f;
  glm::vec3 m_scale;

  // Axis-Aligned Bounding Boxes (cached)
  std::vector<AABB> m_cachedMeshAABBs;
  AABB m_AABB;
};

#endif
