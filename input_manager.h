#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include "camera.h"
#include <GLFW/glfw3.h>

class InputManager {
public:
  InputManager(Camera *camera, float sensitivity = 0.05f);

  // Initialize callbacks with GLFW window
  void SetupCallbacks(GLFWwindow *window);

  // Update input state (call every frame)
  glm::vec3 ProcessKeyboard(GLFWwindow *window, float deltaTime,
                            float velocity);
  bool ProcessJump(GLFWwindow *window);

  // Getters/Setters
  void SetSensitivity(float sensitivity) { this->sensitivity = sensitivity; }
  float GetSensitivity() const { return sensitivity; }

  // For GLFW callbacks (these need to be static)
  static void MouseCallback(GLFWwindow *window, double xpos, double ypos);
  static void KeyCallback(GLFWwindow *window, int key, int scancode, int action,
                          int mods);

private:
  Camera *camera;
  float sensitivity;

  // Mouse state
  static float lastMouseX;
  static float lastMouseY;
  static bool firstMouse;

  // Reference to instance for callbacks
  static InputManager *instance;
};

#endif
