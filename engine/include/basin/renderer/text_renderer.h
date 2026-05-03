#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H

#include <map>
#include <string>
#include "basin/renderer/shader.h"

struct Character {
    unsigned int TextureID;  // ID handle of the glyph texture
    glm::ivec2   Size;       // Size of glyph
    glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
    unsigned int Advance;    // Offset to advance to next glyph
};

class TextRenderer {
public:
    TextRenderer(unsigned int width, unsigned int height);
    ~TextRenderer();
    
    void Load(std::string font, unsigned int fontSize);
    void RenderText(std::string text, float x, float y, float scale, glm::vec3 color);
    void UpdateProjection(unsigned int width, unsigned int height);
    
private:
    std::map<char, Character> Characters;
    Shader* textShader;
    unsigned int VAO, VBO;
    unsigned int screenWidth, screenHeight;
};

#endif
