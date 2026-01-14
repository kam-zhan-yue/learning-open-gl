#include "learnopengl/shapes.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stbi_image.h>
#include <iostream>
#include <array>

using std::string;

// Data Structures
struct WorldData {
  glm::vec3 lightPositions[4];
};

// Function Headers
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window, float &deltaTime);
unsigned int loadTexture(char const *path);
std::array<unsigned int, 2> setupTextures();
void renderModel(Model &model, Shader &shader, WorldData world);
void renderLight(Shader &shader, unsigned int VAO, WorldData world);
void mouseCallback(GLFWwindow *window, double xPos, double yPos);
void scrollCallback(GLFWwindow *window, double xPos, double yPos);
void debugFramebufferTexture(unsigned int texture);

// Global Variables
bool firstMouse = false;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float lastX = 400.0f;
float lastY = 300.0f;

bool initialised = false;
unsigned int shaderDisplayFBOOutput;
unsigned int vaoDebugTexturedRect;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
glm::vec3 lightPos(2.0f, 1.0f, 2.0f);

int windowWidth;
int windowHeight;

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

  glEnable(GL_DEPTH_TEST);

  // Hide cursor and register cursor callback
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(window, mouseCallback);
  glfwSetScrollCallback(window, scrollCallback);

  // NOTE(ALEX): On High DPI Displays, the logical screen size is not the same as the window screen size.
  // This ensures that we have the most accurate screen size after we've created the window
  glfwGetFramebufferSize(window, &windowWidth, &windowHeight);

  return window;
}

int main() {
  GLFWwindow *window = init();

  std::array<unsigned int, 2> textures = setupTextures();

  WorldData world = {
    {
      glm::vec3( 0.7f,  0.2f,  2.0f),
      glm::vec3( 2.3f, -3.3f, -4.0f),
      glm::vec3(-4.0f,  2.0f, -12.0f),
      glm::vec3( 0.0f,  0.0f, -3.0f)
    },
  };

  Shader backpackShader = Shader((string(SHADER_DIR) + "/model-vertex.glsl").c_str(), (string(SHADER_DIR) + "/model-fragment.glsl").c_str());
  Model backpack = Model("/objects/backpack/backpack.obj");

  // Framebuffer Setup
  unsigned int framebufferTexture;
  glGenTextures(1, &framebufferTexture);
  glBindTexture(GL_TEXTURE_2D, framebufferTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  unsigned int FBO, RBO;
  glGenFramebuffers(1, &FBO);
  glGenRenderbuffers(1, &FBO);
  glBindFramebuffer(GL_FRAMEBUFFER, FBO);
  glBindRenderbuffer(GL_RENDERBUFFER, FBO);

  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, windowWidth, windowHeight);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTexture, 0);
  glFramebufferRenderbuffer(GL_RENDERBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, RBO);

  // check if the framebuffer is complete
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  glBindTexture(GL_TEXTURE_2D, 0);

  Shader framebufferShader = Shader(
    (string(SHADER_DIR) + "/framebuffer-debug.vert").c_str(), 
    (string(SHADER_DIR) + "/framebuffer-debug.frag").c_str()
  );

  Quad quad = Quad();

  // Create a render loop that swaps the front/back buffers and polls for user events
  // Necessary to prevent the window from closing instantly
  while (!glfwWindowShouldClose(window)) {
    // delta time calculations
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // inputs
    processInput(window, deltaTime);

    // draw to framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
      glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      renderModel(backpack, backpackShader, world);

    // draw to screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
      glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      renderModel(backpack, backpackShader, world);

    // check events and swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // Terminate and clean up all resources glfwTerminate();
  return 0;
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
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
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

std::array<unsigned int, 2> setupTextures() {
  // generate and bind the texture
  std::array<unsigned int, 2> textures{};

  // flip images vertically
  stbi_set_flip_vertically_on_load(true);

  textures[0] = loadTexture("/textures/container2.png");
  textures[1] = loadTexture("/textures/container2_specular.png");

  return textures;
}

void sendLightData(Shader &shader, WorldData world) {
  // Light Colours and Direction
  glm::vec3 lightColour = glm::vec3(1.0);
  glm::vec3 ambientColour = lightColour * 0.2f;
  glm::vec3 diffuseColour = lightColour * 0.5f;
  glm::vec3 lightDirection = glm::vec3(-0.2f, -1.0f, -0.3f);

  shader.setVec3("viewPos", camera.cameraPos);
  
  // Directional Light Parameters
  shader.setVec3("directionalLight.direction", lightDirection);
  shader.setVec3("directionalLight.ambient", ambientColour);
  shader.setVec3("directionalLight.diffuse", diffuseColour);
  shader.setVec3("directionalLight.specular", glm::vec3(1.0f));

  // Point Light Parameters
  shader.setVec3("pointLights[0].position", world.lightPositions[0]);
  shader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
  shader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
  shader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
  shader.setFloat("pointLights[0].constant", 1.0f);
  shader.setFloat("pointLights[0].linear", 0.09f);
  shader.setFloat("pointLights[0].quadratic", 0.032f);
  // point light 2
  shader.setVec3("pointLights[1].position", world.lightPositions[1]);
  shader.setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
  shader.setVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
  shader.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
  shader.setFloat("pointLights[1].constant", 1.0f);
  shader.setFloat("pointLights[1].linear", 0.09f);
  shader.setFloat("pointLights[1].quadratic", 0.032f);
  // point light 3
  shader.setVec3("pointLights[2].position", world.lightPositions[2]);
  shader.setVec3("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
  shader.setVec3("pointLights[2].diffuse", 0.8f, 0.8f, 0.8f);
  shader.setVec3("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
  shader.setFloat("pointLights[2].constant", 1.0f);
  shader.setFloat("pointLights[2].linear", 0.09f);
  shader.setFloat("pointLights[2].quadratic", 0.032f);
  // point light 4
  shader.setVec3("pointLights[3].position", world.lightPositions[3]);
  shader.setVec3("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
  shader.setVec3("pointLights[3].diffuse", 0.8f, 0.8f, 0.8f);
  shader.setVec3("pointLights[3].specular", 1.0f, 1.0f, 1.0f);
  shader.setFloat("pointLights[3].constant", 1.0f);
  shader.setFloat("pointLights[3].linear", 0.09f);
  shader.setFloat("pointLights[3].quadratic", 0.032f);


  // Spot Light Parameters
  shader.setVec3("spotLight.position", camera.cameraPos);
  shader.setVec3("spotLight.direction", camera.cameraFront);
  shader.setFloat("spotLight.inner", glm::cos(glm::radians(10.5f)));
  shader.setFloat("spotLight.outer", glm::cos(glm::radians(15.0f)));
  shader.setVec3("spotLight.ambient", ambientColour);
  shader.setVec3("spotLight.diffuse", diffuseColour);
  shader.setVec3("spotLight.specular", glm::vec3(1.0f));
  shader.setFloat("spotLight.constant", 1.0f);
  shader.setFloat("spotLight.linear", 0.09f);
  shader.setFloat("spotLight.quadratic", 0.032f);
}

void renderModel(Model &model, Shader &shader, WorldData world) {
  shader.use();

  glm::mat4 view = camera.getLookAt();
  glm::mat4 projection = glm::perspective(glm::radians(camera.zoom), 800.0f / 600.0f, 0.1f, 100.0f);
  glm::mat4 modelMatrix = glm::mat4(1.0);

  shader.setMat4("view", view);
  shader.setMat4("projection", projection);
  shader.setMat4("model", modelMatrix);

  sendLightData(shader, world);

  model.draw(shader);
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

void debugFramebufferTexture(unsigned int texture) {
  if (!initialised) {
    initialised = true;
    shaderDisplayFBOOutput = glCreateProgram();
    float vertices[] = {
      // positions        // texture Coords
      -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
      -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
       1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
       1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    };

    unsigned int VAO, VBO;
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(VAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    vaoDebugTexturedRect = VAO;
  }

  glActiveTexture(GL_TEXTURE0);
  glUseProgram(shaderDisplayFBOOutput);
  glBindTexture(GL_TEXTURE_2D, texture);
  glBindVertexArray(vaoDebugTexturedRect);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindVertexArray(0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glUseProgram(0);
}
