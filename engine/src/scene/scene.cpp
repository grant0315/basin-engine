#include "basin/scene/scene.h"
#include "basin/renderer/model.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <functional>

using basin::Light;
using basin::LightType;

Scene::Scene() : m_name(""), m_camera({glm::vec3(0, 10, 20), 45.0f}), m_spawnPoint(glm::vec3(0, 5, 0)) {}

void Scene::cleanup() {
  for (Entity* ent : m_entities) {
    delete ent;
  }
  m_entities.clear();

  for (Collection* col : m_rootCollections) {
    delete col;
  }
  m_rootCollections.clear();
}

void Scene::resetToEmpty(const std::string& name) {
  cleanup();
  m_lights.clear();
  m_rootCollections.clear();
  m_name = name;
  m_filepath.clear();
  m_lastModifiedTime = std::filesystem::file_time_type();
  m_camera = {glm::vec3(0, 10, 20), 45.0f};
  m_spawnPoint = glm::vec3(0, 5, 0);
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

    // Clear existing entities before loading new ones
    cleanup();

    // Load lights
    m_lights.clear();
    if (j.contains("lights")) {
      for (const auto& lj : j["lights"]) {
        Light light;
        light.name = lj.value("name", "Light");
        std::string typeStr = lj.value("type", "directional");
        if (typeStr == "directional") light.type = LightType::Directional;
        else if (typeStr == "point") light.type = LightType::Point;
        else if (typeStr == "spot") light.type = LightType::Spot;

        if (lj.contains("position")) {
          auto& p = lj["position"];
          light.position = glm::vec3(p[0], p[1], p[2]);
        }
        if (lj.contains("direction")) {
          auto& d = lj["direction"];
          light.direction = glm::vec3(d[0], d[1], d[2]);
        }
        if (lj.contains("color")) {
          auto& c = lj["color"];
          light.color = glm::vec3(c[0], c[1], c[2]);
        }
        light.intensity = lj.value("intensity", 1.0f);
        light.constant = lj.value("constant", 1.0f);
        light.linear = lj.value("linear", 0.09f);
        light.quadratic = lj.value("quadratic", 0.032f);
        light.cutoff = lj.value("cutoff", 12.5f);
        light.outerCutoff = lj.value("outer_cutoff", 17.5f);

        m_lights.push_back(light);
        std::cout << "Loaded light: " << light.name << " (" << typeStr << ")" << std::endl;
      }
    }

    if (j.contains("entities")) {
      for (const auto& ent : j["entities"]) {
        std::string name = ent.value("name", "unnamed");
        std::string type = ent.value("type", "model");
        
        Model* model = nullptr;
        
        PrimitiveParams primParams;
        if (type == "primitive") {
          std::string primitiveType = ent.value("primitive_type", "");
          primParams.primitiveType = primitiveType;

          if (primitiveType == "plane") {
            primParams.width = ent.value("width", 10.0f);
            primParams.depth = ent.value("depth", 10.0f);
            primParams.thickness = ent.value("thickness", 1.0f);
            model = m_primGen.generatePlane(primParams.width, primParams.depth, primParams.thickness);
          } else if (primitiveType == "cube") {
            primParams.size = ent.value("size", 1.0f);
            model = m_primGen.generateCube(primParams.size);
          } else if (primitiveType == "cuboid") {
            primParams.length = ent.value("length", 1.0f);
            primParams.width = ent.value("width", 1.0f);
            primParams.height = ent.value("height", 1.0f);
            model = m_primGen.generateCuboid(primParams.length, primParams.width, primParams.height);
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
        entity->setVisible(ent.value("visible", true));
        if (type == "primitive") {
          entity->setPrimitiveParams(primParams);
        } else {
          ModelParams modelParams;
          modelParams.modelPath = ent.value("model", "");
          modelParams.texturesFolder = ent.value("textures_folder", "");
          if (ent.contains("textures")) {
            auto& tex = ent["textures"];
            modelParams.baseColor = tex.value("base_color", "");
            modelParams.normal = tex.value("normal", "");
            modelParams.height = tex.value("height", "");
            modelParams.roughness = tex.value("roughness", "");
          }
          entity->setModelParams(modelParams);
        }

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

    // Load collections (backward compatible: missing key = no collections)
    if (j.contains("collections")) {
      std::function<void(const json&, Collection*)> loadCollectionTree =
          [&](const json& colArray, Collection* parent) {
        for (const auto& colJson : colArray) {
          std::string colName = colJson.value("name", "Collection");
          bool colVisible = colJson.value("visible", true);
          Collection* col = addCollection(colName, parent);
          col->setVisible(colVisible);

          // Associate entities by name
          if (colJson.contains("entity_names")) {
            for (const auto& entName : colJson["entity_names"]) {
              std::string name = entName.get<std::string>();
              for (Entity* ent : m_entities) {
                if (ent->getName() == name) {
                  col->addEntity(ent);
                  break;
                }
              }
            }
          }

          // Recurse into children
          if (colJson.contains("children")) {
            loadCollectionTree(colJson["children"], col);
          }
        }
      };
      loadCollectionTree(j["collections"], nullptr);
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

void Scene::removeEntity(size_t index) {
  if (index >= m_entities.size()) return;
  Entity* ent = m_entities[index];
  if (ent->getCollection()) {
    ent->getCollection()->removeEntity(ent);
  }
  delete ent;
  m_entities.erase(m_entities.begin() + index);
}

void Scene::swapEntities(size_t a, size_t b) {
  if (a < m_entities.size() && b < m_entities.size() && a != b) {
    std::swap(m_entities[a], m_entities[b]);
  }
}

Collection* Scene::addCollection(const std::string& name, Collection* parent) {
  Collection* col = new Collection(name);
  if (parent) {
    parent->addChild(col);
  } else {
    m_rootCollections.push_back(col);
  }
  return col;
}

void deleteCollectionRecursive(Collection* collection) {
  for (Collection* child : collection->getChildren()) {
    deleteCollectionRecursive(child);
  }
  delete collection;
}

void Scene::removeCollection(Collection* collection) {
  if (!collection) return;

  std::vector<Entity*> entities = collection->collectAllEntities();
  for (Entity* ent : entities) {
    ent->m_collection = nullptr;
  }

  if (collection->getParent()) {
    collection->getParent()->removeChild(collection);
  } else {
    auto it = std::find(m_rootCollections.begin(), m_rootCollections.end(), collection);
    if (it != m_rootCollections.end()) {
      m_rootCollections.erase(it);
    }
  }
  deleteCollectionRecursive(collection);
}

Collection* Scene::findCollection(const std::string& name) {
  std::function<Collection*(const std::vector<Collection*>&)> search =
      [&](const std::vector<Collection*>& collections) -> Collection* {
    for (Collection* col : collections) {
      if (col->getName() == name) return col;
      Collection* found = search(col->getChildren());
      if (found) return found;
    }
    return nullptr;
  };
  return search(m_rootCollections);
}

std::vector<Collection*> Scene::getAllCollections() const {
  std::vector<Collection*> result;
  std::function<void(const std::vector<Collection*>&)> collect =
      [&](const std::vector<Collection*>& collections) {
    for (Collection* col : collections) {
      result.push_back(col);
      collect(col->getChildren());
    }
  };
  collect(m_rootCollections);
  return result;
}

bool Scene::saveToFile(const std::string& filepath) {
  json j;
  j["scene_name"] = m_name;
  j["camera"]["position"] = {m_camera.position.x, m_camera.position.y, m_camera.position.z};
  j["camera"]["fov"] = m_camera.fov;
  j["spawn_point"] = {m_spawnPoint.x, m_spawnPoint.y, m_spawnPoint.z};

  // Serialize lights
  json lights = json::array();
  for (const Light& light : m_lights) {
    json lj;
    lj["name"] = light.name;
    if (light.type == LightType::Directional) lj["type"] = "directional";
    else if (light.type == LightType::Point) lj["type"] = "point";
    else if (light.type == LightType::Spot) lj["type"] = "spot";

    lj["position"] = {light.position.x, light.position.y, light.position.z};
    lj["direction"] = {light.direction.x, light.direction.y, light.direction.z};
    lj["color"] = {light.color.r, light.color.g, light.color.b};
    lj["intensity"] = light.intensity;
    lj["constant"] = light.constant;
    lj["linear"] = light.linear;
    lj["quadratic"] = light.quadratic;
    lj["cutoff"] = light.cutoff;
    lj["outer_cutoff"] = light.outerCutoff;
    lights.push_back(lj);
  }
  j["lights"] = lights;

  json entities = json::array();
  for (Entity* ent : m_entities) {
    json ej;
    ej["name"] = ent->getName();
    ej["position"] = {ent->getPosition().x, ent->getPosition().y, ent->getPosition().z};

    glm::vec3 euler = glm::degrees(glm::eulerAngles(ent->getRotation()));
    ej["rotation"] = {euler.x, euler.y, euler.z};

    ej["scale"] = {ent->getScale().x, ent->getScale().y, ent->getScale().z};
    ej["is_collidable"] = ent->isCollidable();

    glm::vec4 color = ent->getColor();
    ej["color"] = {
      static_cast<int>(color.r * 255),
      static_cast<int>(color.g * 255),
      static_cast<int>(color.b * 255)
    };
    ej["visible"] = ent->isVisible();

    if (ent->hasPrimitiveParams()) {
      const PrimitiveParams& p = ent->getPrimitiveParams();
      ej["type"] = "primitive";
      ej["primitive_type"] = p.primitiveType;
      if (p.primitiveType == "plane") {
        ej["width"] = p.width;
        ej["depth"] = p.depth;
        ej["thickness"] = p.thickness;
      } else if (p.primitiveType == "cube") {
        ej["size"] = p.size;
      } else if (p.primitiveType == "cuboid") {
        ej["length"] = p.length;
        ej["width"] = p.width;
        ej["height"] = p.height;
      }
    } else if (ent->hasModelParams()) {
      const ModelParams& mp = ent->getModelParams();
      ej["type"] = "model";
      ej["model"] = mp.modelPath;
      if (!mp.texturesFolder.empty()) {
        ej["textures_folder"] = mp.texturesFolder;
      }
      json texJson;
      bool hasAnyTexture = false;
      if (!mp.baseColor.empty()) { texJson["base_color"] = mp.baseColor; hasAnyTexture = true; }
      if (!mp.normal.empty()) { texJson["normal"] = mp.normal; hasAnyTexture = true; }
      if (!mp.height.empty()) { texJson["height"] = mp.height; hasAnyTexture = true; }
      if (!mp.roughness.empty()) { texJson["roughness"] = mp.roughness; hasAnyTexture = true; }
      if (hasAnyTexture) {
        ej["textures"] = texJson;
      }
    } else {
      ej["type"] = "model";
    }

    entities.push_back(ej);
  }
  j["entities"] = entities;

  // Serialize collections recursively
  std::function<json(const std::vector<Collection*>&)> serializeCollections =
      [&](const std::vector<Collection*>& collections) -> json {
    json arr = json::array();
    for (Collection* col : collections) {
      json cj;
      cj["name"] = col->getName();
      cj["visible"] = col->isVisible();

      json entNames = json::array();
      for (Entity* ent : col->getEntities()) {
        entNames.push_back(ent->getName());
      }
      cj["entity_names"] = entNames;

      if (!col->getChildren().empty()) {
        cj["children"] = serializeCollections(col->getChildren());
      } else {
        cj["children"] = json::array();
      }
      arr.push_back(cj);
    }
    return arr;
  };

  j["collections"] = serializeCollections(m_rootCollections);

  std::ofstream file(filepath);
  if (!file.is_open()) {
    std::cerr << "ERROR: Could not open file for writing: " << filepath << std::endl;
    return false;
  }
  file << j.dump(2);
  return true;
}