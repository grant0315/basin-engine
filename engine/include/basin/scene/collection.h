#ifndef COLLECTION_H
#define COLLECTION_H

#include <string>
#include <vector>

class Entity;

class Collection {
public:
  Collection(const std::string& name);

  const std::string& getName() const;
  void setName(const std::string& name);

  bool isVisible() const;
  void setVisible(bool visible);
  bool isEffectivelyVisible() const;

  void addEntity(Entity* entity);
  void removeEntity(Entity* entity);
  const std::vector<Entity*>& getEntities() const;

  void addChild(Collection* child);
  void removeChild(Collection* child);
  const std::vector<Collection*>& getChildren() const;

  Collection* getParent() const;
  void setParent(Collection* parent);

  std::vector<Entity*> collectAllEntities() const;

private:
  std::string m_name;
  bool m_visible = true;
  std::vector<Entity*> m_entities;
  std::vector<Collection*> m_children;
  Collection* m_parent = nullptr;
};

#endif