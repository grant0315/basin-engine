#include "basin/scene/scene.h"
#include "basin/renderer/model.h"
#include <fstream>
#include <iostream>

Scene::Scene() : m_name(""), m_camera({glm::vec3(0, 10, 20), 45.0f}), m_spawnPoint(glm::vec3(0, 5, 0)) {}

void Scene::cleanup() {
  for (Entity* ent : m_entities) {
    delete ent;
  }
  m_entities.clear();
}

bool Scene::checkForChanges() {
  try {
    auto currentModTime = std::filesystem::last_write_time(m_filepath);
    if (currentModTime != m_lastModifiedTime) {
      m_lastModifiedTime = currentModTime;
      return true;
    }
  } catch (const std::filesystem::filesystem_error& e) {
    std::cout << "ERROR: Could not check file: " << e.what() << std::endl;
  }
  return false;
}

bool Scene::hotReloadIfChanged() {
  if (!checkForChanges())
    return false;
  return reload();
}

bool Scene::reload() {
  std::cout << "Reloading scene from: " << m_filepath << std::endl;
  cleanup();
  return loadFromFile(m_filepath);
}

bool Scene::loadFromFile(const std::string& filepath) {
  m_filepath = filepath;

  std::ifstream file(filepath);
  if (!file.is_open()) {
    std::cout << "ERROR: Could not open scene file: " << filepath << std::endl;
    return false;
  }

  // Track file modification time
  try {
    m_lastModifiedTime = std::filesystem::last_write_time(filepath);
  } catch (const std::filesystem::filesystem_error& e) {
    std::cout << "WARNING: Could not get file mod time: " << e.what() << std::endl;
  }

  try {
    json j;
    file >> j;

    m_name = j.value("scene_name", "Unnamed");

    if (j.contains("camera")) {
      auto& cam = j["camera"];
      std::vector<float> pos = cam.value("position", std::vector<float>{0, 10, 20});
      m_camera.position = glm::vec3(pos[0], pos[1], pos[2]);
      m_camera.fov = cam.value("fov", 45.0f);
    }

    if (j.contains("spawn_point")) {
      std::vector<float> spawn = j["spawn_point"];
      m_spawnPoint = glm::vec3(spawn[0], spawn[1], spawn[2]);
    }

    if (j.contains("entities")) {
      for (const auto& ent : j["entities"]) {
        std::string name = ent.value("name", "unnamed");
        std::string type = ent.value("type", "model");
        
        Model* model = nullptr;
        
        if (type == "primitive") {
          std::string primitiveType = ent.value("primitive_type", "");
          
          if (primitiveType == "plane") {
            float width = ent.value("width", 10.0f);
            float depth = ent.value("depth", 10.0f);
            float thickness = ent.value("thickness", 1.0f);
            model = m_primGen.generatePlane(width, depth, thickness);
          } else if (primitiveType == "cube") {
            float size = ent.value("size", 1.0f);
            model = m_primGen.generateCube(size);
          } else if (primitiveType == "cuboid") {
            float length = ent.value("length", 1.0f);
            float width = ent.value("width", 1.0f);
            float height = ent.value("height", 1.0f);
            model = m_primGen.generateCuboid(length, width, height);
          } else {
            std::cout << "WARNING: Unknown primitive type '" << primitiveType << "' for entity '" << name << "', skipping." << std::endl;
            continue;
          }
        } else {
          std::string modelPath = ent.value("model", "");
          std::string texturesFolder = ent.value("textures_folder", "");

          ModelTextures textures;
          if (ent.contains("textures")) {
            auto& tex = ent["textures"];
            textures.baseColor = tex.value("base_color", "");
            textures.normal = tex.value("normal", "");
            textures.height = tex.value("height", "");
            textures.roughness = tex.value("roughness", "");
          }

          if (modelPath.empty()) {
            std::cout << "WARNING: Entity '" << name << "' has no model path, skipping." << std::endl;
            continue;
          }

          model = new Model(modelPath, texturesFolder, textures);
        }
        
        std::vector<float> pos = ent.value("position", std::vector<float>{0, 0, 0});
        std::vector<float> rot = ent.value("rotation", std::vector<float>{0, 0, 0});
        std::vector<float> scale = ent.value("scale", std::vector<float>{1, 1, 1});
        bool isCollidable = ent.value("is_collidable", false);

        glm::vec3 position(pos[0], pos[1], pos[2]);
        glm::quat rotation = glm::quat(glm::radians(glm::vec3(rot[0], rot[1], rot[2])));
        glm::vec3 scaleVec(scale[0], scale[1], scale[2]);

        Entity* entity = new Entity(name, model, position, rotation, scaleVec, isCollidable);

        // Override base color from JSON if provided (RGBA 0-255)
        if (ent.contains("color")) {
          auto& c = ent["color"];
          if (c.is_array() && c.size() >= 3) {
            float r = c[0].get<float>() / 255.0f;
            float g = c[1].get<float>() / 255.0f;
            float b = c[2].get<float>() / 255.0f;
            float a = (c.size() >= 4) ? c[3].get<float>() / 255.0f : 1.0f;
            entity->setColor(glm::vec4(r, g, b, a));
          }
        }

        m_entities.push_back(entity);

        std::cout << "Loaded entity: " << name << " (" << type << ")" << std::endl;
      }
    }

    std::cout << "Scene loaded: " << m_name << " with " << m_entities.size() << " entities" << std::endl;
    return true;

  } catch (const json::parse_error& e) {
    std::cout << "ERROR: JSON parse error: " << e.what() << std::endl;
    return false;
  } catch (const std::exception& e) {
    std::cout << "ERROR: " << e.what() << std::endl;
    return false;
  }
}