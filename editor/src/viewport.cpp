#include "viewport.h"
#include <iostream>

namespace basin {

Viewport::Viewport(int width, int height) : m_width(width), m_height(height) {
  createFramebuffer();
}

Viewport::~Viewport() { destroyFramebuffer(); }

void Viewport::createFramebuffer() {
  glGenFramebuffers(1, &m_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

  // Color attachment texture
  glGenTextures(1, &m_colorTexture);
  glBindTexture(GL_TEXTURE_2D, m_colorTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB,
               GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         m_colorTexture, 0);

  // Renderbuffer for depth and stencil
  glGenRenderbuffers(1, &m_rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width,
                        m_height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                            GL_RENDERBUFFER, m_rbo);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cerr << "ERROR: Viewport framebuffer is not complete!" << std::endl;
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Viewport::destroyFramebuffer() {
  glDeleteFramebuffers(1, &m_fbo);
  glDeleteTextures(1, &m_colorTexture);
  glDeleteRenderbuffers(1, &m_rbo);
}

void Viewport::bind() {
  glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
  glViewport(0, 0, m_width, m_height);
  glEnable(GL_DEPTH_TEST);
}

void Viewport::unbind(int windowWidth, int windowHeight) {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, windowWidth, windowHeight);
}

void Viewport::resize(int width, int height) {
  if (width == m_width && height == m_height)
    return;
  m_width = width;
  m_height = height;
  destroyFramebuffer();
  createFramebuffer();
}

} // namespace basin
