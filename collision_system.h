#ifndef COLLISION_SYSTEM_H
#define COLLISION_SYSTEM_H

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

#include "entity.h"

class Player; // Forward declaration to avoid circular include

struct CollisionPair {
  Entity *entityA;
  Entity *entityB;
};

class CollisionSystem {
public:
  CollisionSystem() = default;

  // Rebuild the internal collidable entity list from scene entities
  void rebuildFromScene(Player &player,
                        const std::vector<Entity *> &sceneEntities);

  // Test if two AABBs overlap
  bool checkAABBCollision(const AABB &a, const AABB &b);

  // Broad-phase: check all entity pairs (entity-level AABB)
  std::vector<CollisionPair> checkCollisions();

  // Per-mesh collision resolution for player movement.
  // Tests desired movement on each axis independently, blocks axes that
  // collide, and resets vertical velocity when grounded.
  // Returns the resolved position the player should be placed at.
  glm::vec3 resolveMovement(Player &player,
                            const std::vector<Entity *> &sceneEntities);

private:
  std::vector<Entity *> m_entityCollection;

  // Check if any mesh AABB from the player overlaps any mesh AABB from the
  // scene entities along a single test position
  bool checkMeshCollisionAtPosition(Player &player, glm::vec3 testPos,
                                    const std::vector<Entity *> &sceneEntities);
};

#endif
