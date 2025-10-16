#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include <iostream>

void translation();
void rotate_and_scale();

int main() {
  translation();
  rotate_and_scale();
}

void translation() {
  // Translation Example
  glm::vec4 vec(1.0f, 0.0f, 0.0f, 1.0f);
  glm::mat4 trans = glm::mat4(1.0f);
  trans = glm::translate(trans, glm::vec3(1.0, 1.0, 0.0f));
  std::cout << "=====TRANSLATION=====" << std::endl;
  std::cout << glm::to_string(trans) << std::endl;
  vec = trans * vec;
  std::cout << glm::to_string(vec) << std::endl;
}

void rotate_and_scale() {
  // Rotation and Scale Example
  glm::vec4 vec(1.0f, 0.0f, 0.0f, 1.0f);
  glm::mat4 trans = glm::mat4(1.0f);
  trans = glm::rotate(trans, glm::radians(90.0f), glm::vec3(0.0, 0.0, 1.0));
  trans = glm::scale(trans, glm::vec3(0.5, 0.5, 0.5));
  vec = trans * vec;
  std::cout << "=====ROTATE AND SCALE=====" << std::endl;
  std::cout << glm::to_string(vec) << std::endl;
}
