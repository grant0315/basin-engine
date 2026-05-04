#ifndef TEXTURE_LOADER_H
#define TEXTURE_LOADER_H

#include <string>

unsigned int loadTextureFromFile(const char* path);
unsigned int loadTextureFromMemory(const struct aiTexture* embeddedTex);

#endif