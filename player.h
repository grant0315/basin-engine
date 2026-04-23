#include <glm/gtc/matrix_transform.hpp>

#include "camera.h"
#include "entity.h"
#include "input_manager.h"

class Player : public Entity {
public:
  Player(Model *model, bool isCollidable, const std::string &name = "player")
      : Entity(name, model, isCollidable),
        m_camera(glm::vec3(0.0f, 0.0f, 0.0f)), m_inputManager(&m_camera),
        m_cameraOffset(glm::vec3(0.0f, 0.0f, 0.0f)), m_verticalVelocity(0.0f) {
    setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
  }

  Player(Model *model, bool isCollidable, glm::vec3 cameraOffset,
         const std::string &name = "player")
      : Entity(name, model, isCollidable),
        m_camera(glm::vec3(0.0f, 0.0f, 0.0f)), m_inputManager(&m_camera),
        m_cameraOffset(cameraOffset), m_verticalVelocity(0.0f) {
    setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
  }

  void handleInput(GLFWwindow *window, float deltaTime) {
    // Check for jump BEFORE gravity (so we can jump if grounded)
    bool jumpPressed = m_inputManager.ProcessJump(window);

    // Only jump if pressed NOW and wasn't pressed LAST frame (prevents holding
    // space)
    if (jumpPressed && !m_wasJumpPressed && m_grounded) {
      m_verticalVelocity = m_jumpVelocity;
      m_grounded = false;
    }

    // Track jump state for next frame
    m_wasJumpPressed = jumpPressed;

    // Apply gravity to vertical velocity
    m_verticalVelocity -= m_gravConstant * deltaTime;

    // Get horizontal movement from WASD
    glm::vec3 deltaMovement =
        m_inputManager.ProcessKeyboard(window, deltaTime, m_velocity);
    glm::vec3 worldMovement = (deltaMovement.z * m_camera.GetFront()) +
                              (deltaMovement.x * m_camera.GetRight());

    // Combine horizontal movement with vertical velocity
    worldMovement.y = m_verticalVelocity * deltaTime;

    m_desiredPosition = getPosition() + worldMovement;
  }

  void update(float deltaTime) {
    m_camera.SetPosition(getPosition() + m_cameraOffset);
    m_camera.Update(deltaTime);
  }

  void updateCamera(double mouseXPos, double mouseYPos) {}
  glm::mat4 getViewMatrix() { return m_camera.GetViewMatrix(); };

  glm::vec3 getDesiredPosition() { return m_desiredPosition; }
  void resetVerticalVelocity() {
    m_verticalVelocity = 0.0f;
    m_grounded = true;
  }
  Camera *getCamera() { return &m_camera; }
  InputManager *getInputManager() { return &m_inputManager; }

private:
  Camera m_camera;
  InputManager m_inputManager;
  glm::vec3 m_cameraOffset;
  glm::vec3 m_desiredPosition;

  // Player specific variables
  float m_velocity = 8.0f;
  float m_jumpVelocity = 10.0f;
  float m_verticalVelocity = 0.0f;
  float m_gravConstant = 20.0f;
  bool m_wasJumpPressed = false;
  bool m_grounded = false;
};
