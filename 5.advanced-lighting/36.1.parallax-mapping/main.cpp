#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stbi_image.h>

using namespace std;

// Function Headers
unsigned int generateWall();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window, float &deltaTime);
unsigned int loadTexture(char const *path);
void mouseCallback(GLFWwindow *window, double xPos, double yPos);
void scrollCallback(GLFWwindow *window, double xPos, double yPos);

// Global Variables
bool firstMouse = false;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float lastX = 400.0f;
float lastY = 300.0f;
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

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
  GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
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

  // Tell OpenGL the size of the rendering window so that OpenGL knows how we want to display the data and coordinates
  glViewport(0, 0, 800, 600);


  // Register the frame buffer size callback when the user resizes the window
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  // Hide cursor and register cursor callback
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(window, mouseCallback);
  glfwSetScrollCallback(window, scrollCallback);

  return window;
}

int main() {
  GLFWwindow *window = init(); // Configue Global State glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  Shader shader = Shader(
    (string(SHADER_DIR) + "/wall-vertex.glsl").c_str(),
    (string(SHADER_DIR) + "/wall-fragment.glsl").c_str()
  );

  unsigned int brickWallTexture = loadTexture("/textures/brickwall.jpg");
  unsigned int brickWallNormalTexture = loadTexture("/textures/brickwall_normal.jpg");

  vec3 lightPos(0.5f, 1.0f, 0.3f);

  // Shader Configuration
  shader.use();
  shader.setInt("diffuseMap", 0);
  shader.setInt("normalMap", 1);

  unsigned int wallVAO = generateWall();

  // Create a render loop that swaps the front/back buffers and polls for user events
  // Necessary to prevent the window from closing instantly
  while (!glfwWindowShouldClose(window)) {
    // delta time calculations
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // inputs
    processInput(window, deltaTime);

    // Reset the buffer from the previous render!
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, (float)glfwGetTime(), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
    // Render scene
    shader.use();
    shader.setMat4("model", model);
    shader.setMat4("view", camera.getLookAt());
    shader.setMat4("projection", camera.getPerspective());
    shader.setVec3("lightPos", lightPos);
    shader.setVec3("viewPos", camera.cameraPos);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, brickWallTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, brickWallNormalTexture);
    glBindVertexArray(wallVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);


    // check events and swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // Terminate and clean up all resources glfwTerminate();
  return 0;
}

unsigned int generateWall() {
  // positions
  glm::vec3 pos1(-1.0f,  1.0f, 0.0f);
  glm::vec3 pos2(-1.0f, -1.0f, 0.0f);
  glm::vec3 pos3( 1.0f, -1.0f, 0.0f);
  glm::vec3 pos4( 1.0f,  1.0f, 0.0f);
  // texture coordinates
  glm::vec2 uv1(0.0f, 1.0f);
  glm::vec2 uv2(0.0f, 0.0f);
  glm::vec2 uv3(1.0f, 0.0f);  
  glm::vec2 uv4(1.0f, 1.0f);
  // normal vector
  glm::vec3 nm(0.0f, 0.0f, 1.0f);


  // tangents and bitangents
  glm::vec3 edge1, edge2;
  glm::vec2 deltaUV1, deltaUV2;
  glm::vec3 tangent1, bitangent1;
  glm::vec3 tangent2, bitangent2;
  float f;

  // triangle 1
  edge1 = pos2 - pos1;
  edge2 = pos3 - pos1;
  deltaUV1 = uv2 - uv1;
  deltaUV2 = uv3 - uv1;
  f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
  tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
  tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
  tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

  bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
  bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
  bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

  // triangle 2
  edge1 = pos3 - pos1;
  edge2 = pos4 - pos1;
  deltaUV1 = uv3 - uv1;
  deltaUV2 = uv4 - uv1;

  f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
  tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
  tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
  tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

  bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
  bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
  bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

  float wallVertices[] = {
    // positions            // normal         // texcoords  // tangent                          // bitangent
    pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
    pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
    pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

    pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
    pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
    pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
  };

  unsigned int wallVAO, wallVBO;
  glGenVertexArrays(1, &wallVAO);
  glGenBuffers(1, &wallVBO);
  glBindVertexArray(wallVAO);
  glBindBuffer(GL_ARRAY_BUFFER, wallVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(wallVertices), wallVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
  glEnableVertexAttribArray(4);
  glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  return wallVAO;
}

/*
* Utility function for loading a 2D texture from a file
*/
unsigned int loadTexture(char const *path) {
  unsigned int textureID;
  glGenTextures(1, &textureID);
  
  int width, height, nrChannels;
  std::string resourcePath = (std::string(RESOURCES_DIR) + path);
  unsigned char *data = stbi_load(resourcePath.c_str(), &width, &height, &nrChannels, 0);

  if (data) {
    GLenum format;
    if (nrChannels == 1)
      format = GL_RED;
    else if (nrChannels == 3)
      format = GL_RGB;
    else if (nrChannels == 4)
      format = GL_RGBA;

    // Bind and set the newly created texture
    glBindTexture(GL_TEXTURE_2D, textureID);
    /*glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);*/
    // Force to GL_SRGB
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    // set the texture wrapping / filtering options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // free the data
    stbi_image_free(data);
  } else {
    std::cout << "Failed to load texture" << std::endl;
    stbi_image_free(data);
  }

  return textureID;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
  glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window, float &deltaTime) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }
  camera.process(window, deltaTime);
}

void mouseCallback(GLFWwindow *window, double xPos, double yPos) {
  if (firstMouse) {
    lastX = xPos;
    lastY = yPos;
    firstMouse = false;
  }

  float xOffset = xPos - lastX;
  float yOffset = lastY - yPos;
  lastX = xPos;
  lastY = yPos;

  camera.processMouse(xOffset, yOffset);
}

void scrollCallback(GLFWwindow *window, double xOffset, double yOffset) {
  camera.processScroll(xOffset, yOffset);
}
