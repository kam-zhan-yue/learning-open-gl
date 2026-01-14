#include <glm/gtc/matrix_transform.hpp>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/shapes.h>
#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <stbi_image.h>

// Function Headers
void processInput(GLFWwindow *window);

// Global Variables
const float SCREEN_WIDTH = 800;
const float SCREEN_HEIGHT = 600;

int windowWidth;
int windowHeight;

GLenum glCheckError_(const char *file, int line) {
  GLenum errorCode;
  while ((errorCode = glGetError()) != GL_NO_ERROR) {
    string error;
    switch (errorCode) {
      case GL_INVALID_ENUM: error = "INVALID_ENUM"; break;
      case GL_INVALID_VALUE: error = "INVALID_VALUE"; break;
      case GL_INVALID_OPERATION: error = "INVALID_OPERATION"; break;
      case GL_OUT_OF_MEMORY: error = "OUT_OF_MEMORY"; break;
      case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
    }
    cout << error << " | " << file << " (" << line << ")" << endl;
  }
  return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

GLFWwindow *init() {
  // Init GLFW and set the context variables
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  #ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  #endif

  // Create a window object
  GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "LearnOpenGL", NULL, NULL);
  if (window == NULL) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    throw std::runtime_error("Failed to initialize GLFW");
  }
  glfwMakeContextCurrent(window);

  // Initialise GLAD
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialise GLAD" << std::endl;
    throw std::runtime_error("Failed to initialize GLAD");
  }

  // Hide cursor and register cursor callback
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  // NOTE(ALEX): On High DPI Displays, the logical screen size is not the same as the window screen size.
  // This ensures that we have the most accurate screen size after we've created the window
  glfwGetFramebufferSize(window, &windowWidth, &windowHeight);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

  return window;
}

int main() {
  GLFWwindow *window = init();

  glViewport(0, 0, windowWidth, windowHeight);

  unsigned int tex;
  glBindTexture(GL_TEXTURE_2D, tex);
  glCheckError(); // returns 1282 as the texture isn't generated yet

  glTexImage2D(GL_TEXTURE_3D, 0, GL_RGB, 512, 512, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
  glCheckError(); // returns 1280 as it is an invalid enum

  unsigned int *textures;
  glGenTextures(-5, textures);
  glCheckError(); // returns 1281 as it is an invalid value

  glCheckError(); // returns 0

  glfwTerminate();
  return 0;
}

void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }
}
