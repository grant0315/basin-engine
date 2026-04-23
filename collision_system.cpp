#include "collision_system.h"
#include "player.h"

void CollisionSystem::rebuildFromScene(
    Player &player, const std::vector<Entity *> &sceneEntities) {
  m_entityCollection.clear();
  m_entityCollection.push_back(&player);
  for (Entity *ent : sceneEntities) {
    if (ent->isCollidable()) {
      m_entityCollection.push_back(ent);
    }
  }
}

bool CollisionSystem::checkAABBCollision(const AABB &a, const AABB &b) {
  float aXMin = a.center.x - a.xHalfExtent;
  float aXMax = a.center.x + a.xHalfExtent;
  float bXMin = b.center.x - b.xHalfExtent;
  float bXMax = b.center.x + b.xHalfExtent;

  float aYMin = a.center.y - a.yHalfExtent;
  float aYMax = a.center.y + a.yHalfExtent;
  float bYMin = b.center.y - b.yHalfExtent;
  float bYMax = b.center.y + b.yHalfExtent;

  float aZMin = a.center.z - a.zHalfExtent;
  float aZMax = a.center.z + a.zHalfExtent;
  float bZMin = b.center.z - b.zHalfExtent;
  float bZMax = b.center.z + b.zHalfExtent;

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

bool CollisionSystem::checkMeshCollisionAtPosition(
    Player &player, glm::vec3 testPos,
    const std::vector<Entity *> &sceneEntities) {

  std::vector<AABB> playerAABBs = player.getMeshAABBsAtPosition(testPos);

  for (Entity *ent : sceneEntities) {
    if (!ent->isCollidable())
      continue;

    std::vector<AABB> entMeshAABBs = ent->getMeshAABBs();

    for (const AABB &meshAABB : entMeshAABBs) {
      for (const AABB &pAABB : playerAABBs) {
        if (checkAABBCollision(pAABB, meshAABB)) {
          return true;
        }
      }
    }
  }
  return false;
}

glm::vec3
CollisionSystem::resolveMovement(Player &player,
                                 const std::vector<Entity *> &sceneEntities) {
  glm::vec3 currentPos = player.getPosition();
  glm::vec3 desiredPos = player.getDesiredPosition();

  // Test each axis independently
  glm::vec3 testPosX(desiredPos.x, currentPos.y, currentPos.z);
  glm::vec3 testPosY(currentPos.x, desiredPos.y, currentPos.z);
  glm::vec3 testPosZ(currentPos.x, currentPos.y, desiredPos.z);

  bool xBlocked = checkMeshCollisionAtPosition(player, testPosX, sceneEntities);
  bool yBlocked = checkMeshCollisionAtPosition(player, testPosY, sceneEntities);
  bool zBlocked = checkMeshCollisionAtPosition(player, testPosZ, sceneEntities);

  bool movingUp = desiredPos.y > currentPos.y;

  glm::vec3 resolvedPos(
      xBlocked ? currentPos.x : desiredPos.x,
      // Allow upward movement even with Y collision (jumping), only block
      // downward
      (yBlocked && !movingUp) ? currentPos.y : desiredPos.y,
      zBlocked ? currentPos.z : desiredPos.z);

  if (yBlocked) {
    player.resetVerticalVelocity();
  }

  player.setPosition(resolvedPos);
  return resolvedPos;
}
