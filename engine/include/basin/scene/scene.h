#ifndef SCENE_H
#define SCENE_H

#include "basin/scene/entity.h"
#include "basin/scene/collection.h"
#include "basin/scene/light.h"
#include "basin/math/camera.h"

using basin::Light;
using basin::LightType;
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
  void swapEntities(size_t a, size_t b);

  const std::vector<Collection*>& getRootCollections() const { return m_rootCollections; }
  std::vector<Collection*>& getRootCollections() { return m_rootCollections; }
  Collection* addCollection(const std::string& name, Collection* parent = nullptr);
  void removeCollection(Collection* collection);
  Collection* findCollection(const std::string& name);
  std::vector<Collection*> getAllCollections() const;

  const std::vector<Light>& getLights() const { return m_lights; }
  std::vector<Light>& getLights() { return m_lights; }

  void addLight(const Light& light) { m_lights.push_back(light); }
  void removeLight(size_t index) {
    if (index < m_lights.size()) m_lights.erase(m_lights.begin() + index);
  }

  bool saveToFile(const std::string& filepath);

  std::string getFilepath() const { return m_filepath; }
  void clearFilepath() { m_filepath.clear(); }

  void resetToEmpty(const std::string& name);

private:
  void cleanup();

  std::string m_name;
  std::string m_filepath;
  std::filesystem::file_time_type m_lastModifiedTime;
  SceneCamera m_camera;
  glm::vec3 m_spawnPoint;
  std::vector<Entity*> m_entities;
  std::vector<Light> m_lights;
  std::vector<Collection*> m_rootCollections;
  PrimitiveGenerator m_primGen;
};

#endif