#ifndef BASIN_WINDOW_H
#define BASIN_WINDOW_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace basin {

class Window {
public:
  Window(int width, int height, const char *title);
  ~Window();

  bool shouldClose() const;
  void swapBuffers();
  void pollEvents();

  int getWidth() const { return m_width; }
  int getHeight() const { return m_height; }

  GLFWwindow *getNativeWindow() { return m_window; }

private:
  GLFWwindow *m_window = nullptr;
  int m_width;
  int m_height;

  static void framebufferSizeCallback(GLFWwindow *window, int width,
                                      int height);
};

} // namespace basin

#endif
