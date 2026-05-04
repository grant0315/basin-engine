#include "basin/renderer/material.h"
#include "basin/renderer/texture_loader.h"
#include <glad/glad.h>
#include <iostream>

const char* Material::slotName(TextureSlot slot) {
  switch (slot) {
    case TextureSlot::Diffuse:   return "diffuse";
    case TextureSlot::Normal:    return "normal";
    case TextureSlot::Height:    return "height";
    case TextureSlot::Roughness: return "roughness";
    case TextureSlot::Metallic:  return "metallic";
    case TextureSlot::AO:        return "ao";
    case TextureSlot::Emissive:  return "emissive";
    case TextureSlot::Opacity:   return "opacity";
    default:                     return "unknown";
  }
}

const char* Material::slotFilenameHint(TextureSlot slot) {
  switch (slot) {
    case TextureSlot::Diffuse:   return "Image Files\0*.png;*.jpg;*.jpeg;*.bmp;*.tga;*.tif\0All Files\0*.*\0";
    case TextureSlot::Normal:    return "Normal Maps\0*.png;*.jpg;*.bmp\0All Files\0*.*\0";
    case TextureSlot::Height:    return "Height Maps\0*.png;*.jpg;*.bmp\0All Files\0*.*\0";
    case TextureSlot::Roughness: return "Roughness Maps\0*.png;*.jpg;*.bmp\0All Files\0*.*\0";
    case TextureSlot::Metallic:  return "Metallic Maps\0*.png;*.jpg;*.bmp\0All Files\0*.*\0";
    case TextureSlot::AO:        return "AO Maps\0*.png;*.jpg;*.bmp\0All Files\0*.*\0";
    case TextureSlot::Emissive:  return "Emissive Maps\0*.png;*.jpg;*.bmp\0All Files\0*.*\0";
    case TextureSlot::Opacity:   return "Opacity Maps\0*.png;*.jpg;*.bmp\0All Files\0*.*\0";
    default:                     return "All Files\0*.*\0";
  }
}

void Material::loadTextures(const std::string& texturesFolder) {
  for (int i = 0; i < static_cast<int>(TextureSlot::Count); i++) {
    if (!texturePaths[i].empty() && texturePaths[i] != "none") {
      std::string path = texturePaths[i];
      if (!texturesFolder.empty() && path.find('/') == std::string::npos && path.find('\\') == std::string::npos) {
        path = texturesFolder + "/" + path;
      }
      unsigned int id = loadTextureFromFile(path.c_str());
      if (id != 0) {
        textureIDs[i] = id;
        hasTexture[i] = true;
      } else {
        textureIDs[i] = 0;
        hasTexture[i] = false;
      }
    }
  }
}

void Material::clearTextures() {
  for (int i = 0; i < static_cast<int>(TextureSlot::Count); i++) {
    if (hasTexture[i] && textureIDs[i] != 0) {
      glDeleteTextures(1, &textureIDs[i]);
      textureIDs[i] = 0;
      hasTexture[i] = false;
    }
  }
}

void Material::clearOverrides() {
  for (int i = 0; i < static_cast<int>(TextureSlot::Count); i++) {
    texturePaths[i] = "";
  }
}