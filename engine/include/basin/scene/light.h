#ifndef BASIN_LIGHT_H
#define BASIN_LIGHT_H

#include <glm/glm.hpp>
#include <string>

namespace basin {

enum class LightType { Directional = 0, Point = 1, Spot = 2 };

struct Light {
  std::string name = "Untitled Light";
  LightType type = LightType::Directional;

  glm::vec3 position = glm::vec3(0.0f);
  glm::vec3 direction = glm::vec3(0.0f, -1.0f, 0.0f);
  glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
  float intensity = 1.0f;

  // Attenuation (point / spot)
  float constant = 1.0f;
  float linear = 0.09f;
  float quadratic = 0.032f;

  // Spot angles (degrees)
  float cutoff = 12.5f;
  float outerCutoff = 17.5f;
};

} // namespace basin

#endif
