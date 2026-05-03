#include "basin/platform/window.h"
#include <iostream>

namespace basin {

Window::Window(int width, int height, const char *title)
    : m_width(width), m_height(height) {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  m_window = glfwCreateWindow(width, height, title, NULL, NULL);
  if (!m_window) {
    std::cerr << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return;
  }
  glfwMakeContextCurrent(m_window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << "Failed to initialize GLAD" << std::endl;
    glfwDestroyWindow(m_window);
    glfwTerminate();
    m_window = nullptr;
    return;
  }

  glfwSwapInterval(0);
  glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);

  glViewport(0, 0, width, height);
  glEnable(GL_DEPTH_TEST);

  // Store pointer to this Window instance for the callback
  glfwSetWindowUserPointer(m_window, this);
}

Window::~Window() {
  if (m_window)
    glfwDestroyWindow(m_window);
  glfwTerminate();
}

bool Window::shouldClose() const {
  return glfwWindowShouldClose(m_window);
}

void Window::swapBuffers() { glfwSwapBuffers(m_window); }

void Window::pollEvents() { glfwPollEvents(); }

void Window::setCursorEnabled(bool enabled) {
  glfwSetInputMode(m_window, GLFW_CURSOR,
                   enabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
  if (glfwRawMouseMotionSupported()) {
    glfwSetInputMode(m_window, GLFW_RAW_MOUSE_MOTION,
                     enabled ? GLFW_FALSE : GLFW_TRUE);
  }
}

bool Window::isKeyPressed(int key) const {
  return glfwGetKey(m_window, key) == GLFW_PRESS;
}

bool Window::isMouseButtonPressed(int button) const {
  return glfwGetMouseButton(m_window, button) == GLFW_PRESS;
}

glm::dvec2 Window::getCursorPos() const {
  double x, y;
  glfwGetCursorPos(m_window, &x, &y);
  return glm::dvec2(x, y);
}

void Window::framebufferSizeCallback(GLFWwindow *window, int width,
                                     int height) {
  glViewport(0, 0, width, height);
  Window *self = static_cast<Window *>(glfwGetWindowUserPointer(window));
  if (self) {
    self->m_width = width;
    self->m_height = height;
  }
}

} // namespace basin
