#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

#include "entity.h"

struct CollisionPair {
  Entity *entityA;
  Entity *entityB;
};

class CollisionSystem {
public:
  CollisionSystem(std::vector<Entity *> entityCollection);
  bool checkAABBCollision(const AABB &a, const AABB &b);
  std::vector<CollisionPair> checkCollisions();

private:
  std::vector<Entity *> m_entityCollection;
  bool m_pauseDetection;
};
