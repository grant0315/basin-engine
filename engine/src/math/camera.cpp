#include "basin/math/camera.h"

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : position(position), targetPosition(position), worldUp(up), yaw(yaw),
      pitch(pitch), targetYaw(yaw), targetPitch(pitch), velocity(10.0f) {
  UpdateCameraVectors();
}

void Camera::Update(float deltaTime) {
  // Smooth position interpolation
  position = glm::mix(position, targetPosition, 1.0f - CAMERA_SMOOTHING);

  // Smooth rotation interpolation
  yaw = glm::mix(yaw, targetYaw, 1.0f - CAMERA_SMOOTHING);
  pitch = glm::mix(pitch, targetPitch, 1.0f - CAMERA_SMOOTHING);

  UpdateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix() const {
  return glm::lookAt(position, position + front, up);
}

void Camera::MoveForward(float distance) { targetPosition += front * distance; }

void Camera::MoveBackward(float distance) {
  targetPosition -= front * distance;
}

void Camera::MoveLeft(float distance) { targetPosition -= right * distance; }

void Camera::MoveRight(float distance) { targetPosition += right * distance; }

void Camera::SetPosition(glm::vec3 worldPosition) {
  targetPosition = worldPosition;
}

void Camera::Rotate(float deltaYaw, float deltaPitch) {
  targetYaw += deltaYaw;
  targetPitch += deltaPitch;

  // Constrain pitch to prevent screen flip
  if (targetPitch > 89.0f)
    targetPitch = 89.0f;
  if (targetPitch < -89.0f)
    targetPitch = -89.0f;
}

void Camera::UpdateCameraVectors() {
  // Calculate new front vector
  glm::vec3 newFront;
  newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
  newFront.y = sin(glm::radians(pitch));
  newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
  front = glm::normalize(newFront);

  // Recalculate right and up vectors
  right = glm::normalize(glm::cross(front, worldUp));
  up = glm::normalize(glm::cross(right, front));
}
