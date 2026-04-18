#include "input_manager.h"
#include <iostream>

// Initialize static members
float InputManager::lastMouseX = 0.0f;
float InputManager::lastMouseY = 0.0f;
bool InputManager::firstMouse = true;
InputManager *InputManager::instance = nullptr;

InputManager::InputManager(Camera *camera, float sensitivity)
    : camera(camera), sensitivity(sensitivity) {
  instance = this;
}

void InputManager::SetupCallbacks(GLFWwindow *window) {
  // Get initial window size for mouse centering
  int width, height;
  glfwGetWindowSize(window, &width, &height);
  lastMouseX = width / 2.0f;
  lastMouseY = height / 2.0f;

  // Set GLFW callbacks
  glfwSetCursorPosCallback(window, MouseCallback);
  glfwSetKeyCallback(window, KeyCallback);
}

glm::vec3 InputManager::ProcessKeyboard(GLFWwindow *window, float deltaTime,
                                        float velocity) {
  glm::vec3 deltaMovement(0.0f);
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    deltaMovement.z += velocity * deltaTime;
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    deltaMovement.z -= velocity * deltaTime;
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    deltaMovement.x -= velocity * deltaTime;
  }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    deltaMovement.x += velocity * deltaTime;
  }
  return deltaMovement;
}

bool InputManager::ProcessJump(GLFWwindow *window) {
  return glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
}

void InputManager::MouseCallback(GLFWwindow *window, double xpos, double ypos) {
  if (firstMouse) {
    lastMouseX = xpos;
    lastMouseY = ypos;
    firstMouse = false;
  }

  float deltaX = (xpos - lastMouseX) * instance->sensitivity;
  float deltaY =
      (lastMouseY - ypos) *
      instance
          ->sensitivity; // Reversed since y-coordinates go from bottom to top

  lastMouseX = xpos;
  lastMouseY = ypos;

  instance->camera->Rotate(deltaX, deltaY);
}

void InputManager::KeyCallback(GLFWwindow *window, int key, int scancode,
                               int action, int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    std::cout << "Closing GLFW window" << std::endl;
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
}
