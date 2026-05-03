#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Global camera smoothing factor (0.0 = instant, 1.0 = very smooth)
// Higher values = smoother/slower camera movement
// Recommended range: 0.0 - 0.95
const float CAMERA_SMOOTHING = 0.7f;

class Camera {
public:
  Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
         glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = -90.0f,
         float pitch = 0.0f);

  // Update camera smoothing (call once per frame)
  void Update(float deltaTime);

  // Get view matrix for rendering
  glm::mat4 GetViewMatrix() const;

  // Get camera vectors
  glm::vec3 GetPosition() const { return position; }
  glm::vec3 GetFront() const { return front; }
  glm::vec3 GetUp() const { return up; }
  glm::vec3 GetRight() const { return right; }

  // Get rotation angles
  float GetYaw() const { return yaw; }
  float GetPitch() const { return pitch; }

  // Movement
  void MoveForward(float distance);
  void MoveBackward(float distance);
  void MoveLeft(float distance);
  void MoveRight(float distance);
  void SetPosition(glm::vec3 worldPosition);

  // Rotation
  void Rotate(float deltaYaw, float deltaPitch);

  // Settings
  void SetVelocity(float velocity) { this->velocity = velocity; }
  float GetVelocity() const { return velocity; }

private:
  void UpdateCameraVectors();

  // Camera attributes
  glm::vec3 position;
  glm::vec3 targetPosition; // Target position for smoothing
  glm::vec3 front;
  glm::vec3 up;
  glm::vec3 right;
  glm::vec3 worldUp;

  // Euler angles
  float yaw;
  float pitch;
  float targetYaw;   // Target yaw for smoothing
  float targetPitch; // Target pitch for smoothing

  // Camera settings
  float velocity;
};

#endif
