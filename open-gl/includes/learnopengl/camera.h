#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

class Camera {
public:
  glm::vec3 cameraPos;
  glm::vec3 cameraFront;
  glm::vec3 cameraUp;
  float yaw;
  float pitch;

  float cameraSpeed = 2.5f;
  float sensitivity = 0.1f;
  float zoom = 45.0f;

  Camera(glm::vec3 initialPos) {
    cameraPos = initialPos;
    cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

    // initialise yaw to -90.0f 
    yaw = -90.0f;
    pitch = 0.0f;
  }

  glm::mat4 getLookAt() {
    return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
  }

  glm::mat4 getPerspective() {
    return glm::perspective(glm::radians(zoom), 800.0f / 600.0f, 0.1f, 100.0f);
  }

  void process(GLFWwindow *window, float &deltaTime) {
    const float speed = cameraSpeed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
      cameraPos += speed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
      cameraPos -= speed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
      cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * speed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
      cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * speed;
  }

  void processMouse(double xOffset, double yOffset) {
    yaw += xOffset * sensitivity;
    pitch += yOffset * sensitivity;

    if (pitch > 89.0f)
      pitch = 89.0f;
    if (pitch < -89.0f)
      pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
  }

  void processScroll(double xOffset, double yOffset) {
    zoom -= (float)yOffset;
    if (zoom < 0.01f)
      zoom = 0.01f;
    if (zoom > 45.0f)
      zoom = 45.0f;
  }
};

#endif
