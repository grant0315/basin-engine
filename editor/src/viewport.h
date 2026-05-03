#ifndef BASIN_EDITOR_VIEWPORT_H
#define BASIN_EDITOR_VIEWPORT_H

#include <glad/glad.h>

namespace basin {

class Viewport {
public:
  Viewport(int width, int height);
  ~Viewport();

  void bind();
  void unbind(int windowWidth, int windowHeight);
  void resize(int width, int height);

  unsigned int getColorTexture() const { return m_colorTexture; }
  int getWidth() const { return m_width; }
  int getHeight() const { return m_height; }

private:
  unsigned int m_fbo = 0;
  unsigned int m_colorTexture = 0;
  unsigned int m_rbo = 0;
  int m_width;
  int m_height;

  void createFramebuffer();
  void destroyFramebuffer();
};

} // namespace basin

#endif
