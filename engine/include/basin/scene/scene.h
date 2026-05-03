#ifndef SCENE_H
#define SCENE_H

#include "basin/scene/entity.h"
#include "basin/math/camera.h"
#include "basin/scene/primitive_generator.h"
#include <nlohmann/json.hpp>
#include <filesystem>
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
  bool reload();
  bool checkForChanges();

  // Check for file changes and reload if needed. Returns true if scene was reloaded.
  bool hotReloadIfChanged();
  
  std::string getName() const { return m_name; }
  void setName(const std::string& name) { m_name = name; }

  SceneCamera getCamera() const { return m_camera; }
  glm::vec3 getSpawnPoint() const { return m_spawnPoint; }
  void setSpawnPoint(const glm::vec3& point) { m_spawnPoint = point; }

  const std::vector<Entity*>& getEntities() const { return m_entities; }
  std::vector<Entity*>& getEntities() { return m_entities; }

  void addEntity(Entity* entity) { m_entities.push_back(entity); }
  void removeEntity(size_t index);

  bool saveToFile(const std::string& filepath);

private:
  void cleanup();

  std::string m_name;
  std::string m_filepath;
  std::filesystem::file_time_type m_lastModifiedTime;
  SceneCamera m_camera;
  glm::vec3 m_spawnPoint;
  std::vector<Entity*> m_entities;
  PrimitiveGenerator m_primGen;
};

#endif