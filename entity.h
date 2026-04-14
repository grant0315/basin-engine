#include "model.h"
#include "shader.h"
#include <glad/glad.h>
#include <glm/common.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

class Entity {
public:
  Entity(const std::string &entName, Model *model) {
    m_name = entName;
    m_model = model;

    // Set defaults for Model position
    m_position = glm::vec3(1.0f, 1.0f, 1.0f);
    m_rotation = glm::quat();
    m_scale = glm::vec3(1.0f, 1.0f, 1.0f);

    m_dirty = true; // set flag to ensure model calculation after init

    std::cout << "Initialized Entity: " << m_name << std::endl;
  }

  Entity(const std::string &entName, Model *model, glm::vec3 pos, glm::quat rot,
         glm::vec3 scale) {
    m_name = entName;
    m_model = model;

    // Set defaults for model pos, rot, and scale
    m_position = pos;
    m_rotation = rot;
    m_scale = scale;

    m_dirty = true; // set flag to ensure model calculation after init

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

  glm::vec3 getPosition() const { return m_position; }
  glm::quat getRotation() const { return m_rotation; }

  glm::vec3 getModelCenter() const { return m_model->getModelCenter(); }

  // --- Setters ---
  void setPosition(glm::vec3 pos) {
    m_position = pos;
    m_dirty = true;
  }

  void setScale(glm::vec3 scale) {
    m_scale = scale;
    m_dirty = true;
  }

  void setRotationEuler(glm::vec3 eulerDegrees) {
    m_rotation = glm::quat(glm::radians(eulerDegrees));
    m_dirty = true;
  }

  void setRotationQuat(glm::quat rot) {
    m_rotation = rot;
    m_dirty = true;
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
    // 3. Tell the model to render itself
    glm::mat4 modelMatrix = getModelMatrix();
    shader.setUniform("model", modelMatrix);
    m_model->render();
    m_dirty = false; // Reset dirty flag
  }

private:
  bool m_dirty; // Flag for needing rebuild of matrix

  std::string m_name;
  Model *m_model;
  glm::mat4 m_modelMatrix;

  glm::vec3 m_position;
  glm::quat m_rotation;
  glm::vec3 m_scale;
};
