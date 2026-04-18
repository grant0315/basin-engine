#ifndef SCENE_H
#define SCENE_H

#include "entity.h"
#include "camera.h"
#include "primitive_generator.h"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

using json = nlohmann::json;

struct SceneCamera {
  glm::vec3 position;
  float fov;
};

class Scene {
public:
  Scene();
  bool loadFromFile(const std::string& filepath);
  
  std::string getName() const { return m_name; }
  SceneCamera getCamera() const { return m_camera; }
  glm::vec3 getSpawnPoint() const { return m_spawnPoint; }
  const std::vector<Entity*>& getEntities() const { return m_entities; }

private:
  std::string m_name;
  SceneCamera m_camera;
  glm::vec3 m_spawnPoint;
  std::vector<Entity*> m_entities;
  PrimitiveGenerator m_primGen;
};

#endif