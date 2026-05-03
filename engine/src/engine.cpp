#include "basin/engine.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

namespace basin {

Engine::Engine(int width, int height, const char *title)
    : m_window(width, height, title) {}

void Engine::updateTiming(double currentTime, float &outDeltaTime) {
  outDeltaTime = static_cast<float>(currentTime - m_lastFrameTime);
  m_lastFrameTime = currentTime;

  m_frameCount++;
  if (currentTime - m_lastFpsTime >= 1.0) {
    m_fps = m_frameCount / static_cast<float>(currentTime - m_lastFpsTime);
    m_frameCount = 0;
    m_lastFpsTime = currentTime;
  }
}

void Engine::run(Application *app) {
  if (!app)
    return;

  // Timing init
  double now = glfwGetTime();
  m_lastFpsTime = now;
  m_lastFrameTime = now;

  app->onInit(m_window);

  while (!m_window.shouldClose()) {
    double currentTime = glfwGetTime();
    float deltaTime;
    updateTiming(currentTime, deltaTime);

    app->onUpdate(deltaTime, m_window);
    app->onRender();

    m_window.swapBuffers();
    m_window.pollEvents();
  }

  app->onShutdown();
}

} // namespace basin
