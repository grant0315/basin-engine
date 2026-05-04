#ifndef MATERIAL_H
#define MATERIAL_H

#include <glm/glm.hpp>
#include <string>
#include <array>

enum class TextureSlot : int {
  Diffuse = 0,
  Normal = 1,
  Height = 2,
  Roughness = 3,
  Metallic = 4,
  AO = 5,
  Emissive = 6,
  Opacity = 7,
  Count = 8
};

enum class ShaderType {
  Standard,
  PBR,
  DotMatrix,
  Unlit
};

struct Material {
  glm::vec3 baseColor = glm::vec3(1.0f);
  float roughness = 0.5f;
  float metallic = 0.0f;
  float opacity = 1.0f;
  glm::vec3 emissive = glm::vec3(0.0f);
  float aoStrength = 1.0f;

  std::string texturePaths[static_cast<int>(TextureSlot::Count)];
  unsigned int textureIDs[static_cast<int>(TextureSlot::Count)] = {0};
  bool hasTexture[static_cast<int>(TextureSlot::Count)] = {false};

  ShaderType shaderType = ShaderType::Standard;
  bool isOverride = false;

  void loadTextures(const std::string& texturesFolder = "");
  void clearTextures();
  void clearOverrides();

  static const char* slotName(TextureSlot slot);
  static const char* slotFilenameHint(TextureSlot slot);
};

#endif