#include "collision_system.h"

CollisionSystem::CollisionSystem(std::vector<Entity *> entityCollection) {
  m_entityCollection = entityCollection;
}

bool CollisionSystem::checkAABBCollision(const AABB &a, const AABB &b) {
  // Calculate min/max of x for both A & B AABB
  float aXMin = a.center.x - a.xHalfExtent;
  float aXMax = a.center.x + a.xHalfExtent;
  float bXMin = b.center.x - b.xHalfExtent;
  float bXMax = b.center.x + b.xHalfExtent;
  // Calculate y min/max
  float aYMin = a.center.y - a.yHalfExtent;
  float aYMax = a.center.y + a.yHalfExtent;
  float bYMin = b.center.y - b.yHalfExtent;
  float bYMax = b.center.y + b.yHalfExtent;
  // Caculate z min/max
  float aZMin = a.center.z - a.zHalfExtent;
  float aZMax = a.center.z + a.zHalfExtent;
  float bZMin = b.center.z - b.zHalfExtent;
  float bZMax = b.center.z + b.zHalfExtent;

  // return if all axises overlap between AABB's
  return (aXMax >= bXMin && aXMin <= bXMax && aYMax >= bYMin &&
          aYMin <= bYMax && aZMax >= bZMin && aZMin <= bZMax);
}

std::vector<CollisionPair> CollisionSystem::checkCollisions() {
  std::vector<CollisionPair> collidedEntities;
  for (unsigned int i = 0; i < m_entityCollection.size(); i++) {
    for (unsigned int j = i + 1; j < m_entityCollection.size(); j++) {
      if (checkAABBCollision(
              m_entityCollection[i]->getAxisAlignedBoundingBox(),
              m_entityCollection[j]->getAxisAlignedBoundingBox())) {
        CollisionPair p;
        p.entityA = m_entityCollection[i];
        p.entityB = m_entityCollection[j];
        collidedEntities.push_back(p);
      }
    }
  }
  return collidedEntities;
}
