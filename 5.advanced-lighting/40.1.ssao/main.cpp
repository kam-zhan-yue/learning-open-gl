#include <glm/gtc/matrix_transform.hpp>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h> 
#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <stbi_image.h>
#include <random>

using namespace std;

struct GameObject {
  Shader shader;
  unsigned int VAO;
};

struct Light {
  vec3 position;
  vec3 colour;
  float radius;
};

struct Buffers {
  unsigned int gBuffer;
  unsigned int gPosition;
  unsigned int gNormal;
  unsigned int gColor;
};

struct Shaders {
  Shader geometry;
  Shader screen;
  Shader ssao;
};

struct Vertices {
  unsigned int cube;
  unsigned int quad;
};

struct Models {
  Model backpack;
};

struct SSAO {
  unsigned int ssaoBuffer;
  unsigned int ssaoTexture;
  vector<glm::vec3> kernel;
  unsigned int noiseTexture;
};

struct Scene {
  Shaders shaders;
  Vertices vertices;
  Models models;
  Buffers buffers;
  SSAO ssao;
};

// Function Headers
unsigned int generateCube();
unsigned int generateQuad();
Scene generateScene();
void renderScene(Scene scene);
void renderCube(unsigned int cube);
void renderQuad(unsigned int quad);
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
const float SCREEN_WIDTH = 800;
const float SCREEN_HEIGHT = 600;

int windowWidth;
int windowHeight;

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

  // Register the frame buffer size callback when the user resizes the window
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  // Hide cursor and register cursor callback
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(window, mouseCallback);
  glfwSetScrollCallback(window, scrollCallback);

  // NOTE(ALEX): On High DPI Displays, the logical screen size is not the same as the window screen size.
  // This ensures that we have the most accurate screen size after we've created the window
  glfwGetFramebufferSize(window, &windowWidth, &windowHeight);

  return window;
}

void geometryPass(Scene scene) {
  glBindFramebuffer(GL_FRAMEBUFFER, scene.buffers.gBuffer);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  renderScene(scene);
}

void ssaoPass(Scene scene) {
  Shader ssao = scene.shaders.ssao;
  glBindFramebuffer(GL_FRAMEBUFFER, scene.ssao.ssaoBuffer);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  ssao.use();
  ssao.setInt("positionBuffer", 0);
  ssao.setInt("normalBuffer", 1);
  ssao.setInt("noiseBuffer", 2);
  ssao.setMat4("projection", camera.getPerspective());

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, scene.buffers.gPosition);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, scene.buffers.gNormal);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, scene.ssao.noiseTexture);

  // Send kernel data
  for (unsigned int i=0; i<scene.ssao.kernel.size(); ++i)
    ssao.setVec3("samples[" + to_string(i) + "]", scene.ssao.kernel[i]);

  renderQuad(scene.vertices.quad);
}

void lightingPass(Scene scene) {
  // Deferred Pass
  Shader screen = scene.shaders.screen;
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  screen.use();
  screen.setInt("positionBuffer", 0);
  screen.setInt("normalBuffer", 1);
  screen.setInt("albedoBuffer", 2);
  screen.setInt("ssaoBuffer", 3);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, scene.buffers.gPosition);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, scene.buffers.gNormal);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, scene.buffers.gColor);
  glActiveTexture(GL_TEXTURE3);
  glBindTexture(GL_TEXTURE_2D, scene.ssao.ssaoTexture);

  renderQuad(scene.vertices.quad);
}

void deferredRendering(Scene scene) {
  geometryPass(scene);
  ssaoPass(scene);
  lightingPass(scene);
}

int main() {
  GLFWwindow *window = init(); // Configue Global State glEnable(GL_DEPTH_TEST);
  /*glEnable(GL_BLEND);*/
  /*glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);*/
  glEnable(GL_DEPTH_TEST);

  Scene scene = generateScene();

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
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    deferredRendering(scene);

    // check events and swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // Terminate and clean up all resources glfwTerminate();
  glfwTerminate();
  return 0;
}

Buffers generateBuffers() {
  unsigned int gBuffer;
  glGenFramebuffers(1, &gBuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
  unsigned int gPosition, gNormal, gColor;

  // position colour buffer
  glGenTextures(1, &gPosition);
  glBindTexture(GL_TEXTURE_2D, gPosition);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

  // normal colour buffer
  glGenTextures(1, &gNormal);
  glBindTexture(GL_TEXTURE_2D, gNormal);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

  // colour colour buffer
  glGenTextures(1, &gColor);
  glBindTexture(GL_TEXTURE_2D, gColor);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, windowWidth, windowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gColor, 0);

  // Explicitly tell OpenGL to use two colour attachments
  unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
  glDrawBuffers(3, attachments);

  unsigned int RBO;
  glGenRenderbuffers(1, &RBO);
  glBindRenderbuffer(GL_RENDERBUFFER, RBO);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, windowWidth, windowHeight);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, RBO);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    cout << "Framebuffer is not complete." << endl;

  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindTexture(GL_TEXTURE_2D, 0);

  return { 
    .gBuffer = gBuffer, 
    .gPosition = gPosition, 
    .gNormal = gNormal, 
    .gColor = gColor, 
  };
}

Shaders generateShaders() {
  Shader geometry = Shader(
    (string(SHADER_DIR) + "/geometry-pass-vertex.glsl").c_str(),
    (string(SHADER_DIR) + "/geometry-pass-fragment.glsl").c_str()
  );
  Shader ssao = Shader(
    (string(SHADER_DIR) + "/ssao-vertex.glsl").c_str(),
    (string(SHADER_DIR) + "/ssao-fragment.glsl").c_str()
  );
  Shader screen = Shader(
    (string(SHADER_DIR) + "/screen-vertex.glsl").c_str(),
    (string(SHADER_DIR) + "/screen-fragment.glsl").c_str()
  );
  return { 
    .geometry = geometry,
    .screen = screen,
    .ssao = ssao,
  };
}

Vertices generateVertices() {
  unsigned int cube = generateCube();
  unsigned int quad = generateQuad();
  return { cube, quad };
}

Models generateModels() {
  Model backpack = Model("/objects/backpack/backpack.obj");
  return { backpack };
}

float lerpFloat(float a, float b, float f) {
  return a + f * (b - a);
}

SSAO generateSSAO() {
  uniform_real_distribution<float> randomFloats(0.0, 1.0);
  default_random_engine generator;
  vector<glm::vec3> kernel;
  for (unsigned int i=0; i<64; ++i) {
    glm::vec3 sample(
      randomFloats(generator) * 2.0 - 1.0,
      randomFloats(generator) * 2.0 - 1.0,
      randomFloats(generator)
    );
    sample = glm::normalize(sample);
    sample *= randomFloats(generator);
    float scale = (float)i / 64.0;
    scale = lerpFloat(0.1f, 1.0f, scale * scale);
    sample *= scale;
    kernel.push_back(sample);
  }

  vector<glm::vec3> noises;
  for (unsigned int i=0; i<16; ++i) {
    glm::vec3 sample(
      randomFloats(generator) * 2.0 - 1.0,
      randomFloats(generator) * 2.0 - 1.0,
      0.0
    );
    noises.push_back(sample);
  }

  // Noise texture
  unsigned int noiseTexture;
  glGenTextures(1, &noiseTexture);
  glBindTexture(GL_TEXTURE_2D, noiseTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGB, GL_FLOAT, &noises[0]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glBindTexture(GL_TEXTURE_2D, 0);

  // SSAO Framebuffer
  unsigned int ssaoFBO;
  glGenFramebuffers(1, &ssaoFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
  unsigned int ssaoColourBuffer;
  glGenTextures(1, &ssaoColourBuffer);
  glBindTexture(GL_TEXTURE_2D, ssaoColourBuffer);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, windowWidth, windowHeight, 0, GL_RED, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColourBuffer, 0);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    cout << "Framebuffer is not complete." << endl;

  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindTexture(GL_TEXTURE_2D, 0);

  return {
    .ssaoBuffer = ssaoFBO,
    .ssaoTexture = ssaoColourBuffer,
    .kernel = kernel,
    .noiseTexture = noiseTexture,
  };
}

Scene generateScene() {
  return {
    .shaders = generateShaders(),
    .vertices = generateVertices(),
    .models = generateModels(),
    .buffers = generateBuffers(),
    .ssao = generateSSAO(),
  };
}

void renderScene(Scene scene) {
  Shader geometryPass = scene.shaders.geometry;
  geometryPass.use();
  geometryPass.setMat4("view", camera.getLookAt());
  geometryPass.setMat4("projection", camera.getPerspective());
  glm::mat4 model = glm::mat4(1.0f);

  // render room cube
  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(0.0, 7.0f, 0.0f));
  model = glm::scale(model, glm::vec3(7.5f, 7.5f, 7.5f));
  geometryPass.setMat4("model", model);
  geometryPass.setBool("invertedNormals", true); // invert normals as we're inside the cube
  renderCube(scene.vertices.cube);
  geometryPass.setBool("invertedNormals", false); 

  // render backpack
  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(0.0f, 0.5f, 0.0));
  model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));
  model = glm::scale(model, glm::vec3(1.0f));
  geometryPass.setMat4("model", model);
  scene.models.backpack.draw(geometryPass);
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
  /*glViewport(0, 0, width, height);*/
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

unsigned int generateCube() {
  float vertices[] = {
    // back face
    -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
     1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
     1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
     1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
    -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
    -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
    // front face
    -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
     1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
     1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
     1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
    -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
    -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
    // left face
    -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
    -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
    -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
    -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
    -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
    -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
    // right face
     1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
     1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
     1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
     1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
     1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
     1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
    // bottom face
    -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
     1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
     1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
     1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
    -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
    -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
    // top face
    -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
     1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
     1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
     1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
    -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
    -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
  };

  unsigned int VAO, VBO;
  glGenBuffers(1, &VBO);
  glGenVertexArrays(1, &VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glBindVertexArray(VAO);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  return VAO;
}

unsigned int generateQuad() {
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

  return VAO;
}

void renderQuad(unsigned int quad) {
  glBindVertexArray(quad);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glBindVertexArray(0);
}

void renderCube(unsigned int cube) {
  glBindVertexArray(cube);
  glDrawArrays(GL_TRIANGLES, 0, 36);
  glBindVertexArray(0);
}

