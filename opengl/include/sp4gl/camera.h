#pragma once

#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <vector>

enum Camera_Movement { FORWARD, BACKWARD, LEFT, RIGHT };

class Camera {
public:
  glm::vec3 Position, Front, Up, Right, WorldUp;
  float Yaw, Pitch, MovementSpeed, MouseSensitivity, Zoom;

  Camera(glm::vec3 position = glm::vec3(0.),
         glm::vec3 up = glm::vec3(0., 1., 0.), float yaw = -90.,
         float pitch = 0.)
      : Front(glm::vec3(0., 0., 1.)), MovementSpeed(2.5f),
        MouseSensitivity(0.5), Zoom(45.) {
    Position = position;
    WorldUp = up;
    Yaw = yaw;
    Pitch = pitch;
    updateCameraVectors();
  }

  glm::mat4 GetViewMatrix() {
    return glm::lookAt(Position, Position + Front, Up);
  }

  void ProcessKeyBoard(Camera_Movement direction, float deltaTime) {
    float velocity = MovementSpeed * deltaTime;
    switch (direction) {
    case FORWARD:
      Position += Front * velocity;
      break;
    case BACKWARD:
      Position -= Front * velocity;
      break;
    case LEFT:
      Position -= glm::normalize(glm::cross(Front, WorldUp)) * velocity;
      break;
    case RIGHT:
      Position += glm::normalize(glm::cross(Front, WorldUp)) * velocity;
      break;
    }
  }

  void ProcessMouseMovement(float xoffset, float yoffset,
                            bool constrainPitch = true) {
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw += xoffset;
    Pitch += yoffset;

    if (constrainPitch) {
      if (Pitch > 89.f) {
        Pitch = 89.f;
      }
      if (Pitch < -89.f) {
        Pitch = -89.f;
      }
    }
    updateCameraVectors();
  }

  void ProcessMouseScroll(float yoffset) {
    Zoom -= (float)yoffset;
    if (Zoom < 1.) {
      Zoom = 1.;
    }
    if (Zoom > 45.) {
      Zoom = 45.f;
    }
  }

private:
  void updateCameraVectors() {
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));
  }
};
