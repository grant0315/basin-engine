#include "basin/scene/collection.h"
#include "basin/scene/entity.h"

Collection::Collection(const std::string& name) : m_name(name) {}

const std::string& Collection::getName() const { return m_name; }
void Collection::setName(const std::string& name) { m_name = name; }

bool Collection::isVisible() const { return m_visible; }
void Collection::setVisible(bool visible) { m_visible = visible; }

bool Collection::isEffectivelyVisible() const {
  if (!m_visible) return false;
  if (m_parent) return m_parent->isEffectivelyVisible();
  return true;
}

void Collection::addEntity(Entity* entity) {
  if (!entity) return;
  if (entity->getCollection()) {
    entity->getCollection()->removeEntity(entity);
  }
  m_entities.push_back(entity);
  entity->m_collection = this;
}

void Collection::removeEntity(Entity* entity) {
  if (!entity) return;
  for (auto it = m_entities.begin(); it != m_entities.end(); ++it) {
    if (*it == entity) {
      entity->m_collection = nullptr;
      m_entities.erase(it);
      return;
    }
  }
}

const std::vector<Entity*>& Collection::getEntities() const { return m_entities; }

void Collection::addChild(Collection* child) {
  if (!child) return;
  if (child->m_parent) {
    child->m_parent->removeChild(child);
  }
  m_children.push_back(child);
  child->m_parent = this;
}

void Collection::removeChild(Collection* child) {
  if (!child) return;
  for (auto it = m_children.begin(); it != m_children.end(); ++it) {
    if (*it == child) {
      child->m_parent = nullptr;
      m_children.erase(it);
      return;
    }
  }
}

const std::vector<Collection*>& Collection::getChildren() const { return m_children; }

Collection* Collection::getParent() const { return m_parent; }
void Collection::setParent(Collection* parent) { m_parent = parent; }

std::vector<Entity*> Collection::collectAllEntities() const {
  std::vector<Entity*> result;
  result.insert(result.end(), m_entities.begin(), m_entities.end());
  for (Collection* child : m_children) {
    std::vector<Entity*> childEntities = child->collectAllEntities();
    result.insert(result.end(), childEntities.begin(), childEntities.end());
  }
  return result;
}