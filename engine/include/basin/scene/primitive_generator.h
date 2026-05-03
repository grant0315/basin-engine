#ifndef PRIMITIVE_GENERATOR_H
#define PRIMITIVE_GENERATOR_H

#include "basin/renderer/model.h"

class PrimitiveGenerator {
public:
  Model *generateCube(float size);
  Model *generateCuboid(float length, float width, float height);
  Model *generatePlane(float width, float depth, float thickness = 1.0f);
};

#endif
