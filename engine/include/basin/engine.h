#ifndef BASIN_ENGINE_H
#define BASIN_ENGINE_H

#include "basin/platform/window.h"

namespace basin {

class Application {
public:
  virtual ~Application() = default;

  virtual void onInit(Window &window) {}
  virtual void onUpdate(float deltaTime, Window &window) {}
  virtual void onRender() {}
  virtual void onShutdown() {}
};

class Engine {
public:
  Engine(int width, int height, const char *title);

  void run(Application *app);

  Window &getWindow() { return m_window; }

private:
  Window m_window;

  // Timing
  double m_lastFpsTime = 0.0;
  double m_lastFrameTime = 0.0;
  int m_frameCount = 0;
  float m_fps = 0.0f;

  void updateTiming(double currentTime, float &outDeltaTime);
};

} // namespace basin

#endif
