#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

using std::string;

// Function Headers
glm::mat4 getProjection();
unsigned int loadTexture(char const *path);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window, float &deltaTime);
void mouseCallback(GLFWwindow *window, double xPos, double yPos);
void scrollCallback(GLFWwindow *window, double xPos, double yPos);

// Global Variables
bool firstMouse = false;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float lastX = 400.0f;
float lastY = 300.0f;
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
glm::vec3 lightPos(2.0f, 1.0f, 2.0f);

GLFWwindow *init() {
  // Init GLFW and set the context variables
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  #ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  #endif

  // Hint for multisampling
  glfwWindowHint(GLFW_SAMPLES, 4);

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
  GLFWwindow *window = init();

  // Configure Depth and Stencil Testing Stuff
  // -----------------------------
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_STENCIL_TEST);
  glDepthFunc(GL_LESS);
  glEnable(GL_BLEND);
  glEnable(GL_MULTISAMPLE);

  // Setup Shader
  // -----------
  Shader shader(
    (std::string(SHADER_DIR) + "/vertex.glsl").c_str(),
    (std::string(SHADER_DIR) + "/fragment.glsl").c_str()
  );

  float vertices[] = {
    // back face
    -0.5f, -0.5f, -0.5f,
    0.5f,  0.5f, -0.5f,
    0.5f, -0.5f, -0.5f,
    0.5f,  0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f,
    // front face
    -0.5f, -0.5f,  0.5f,
    0.5f, -0.5f,  0.5f,
    0.5f,  0.5f,  0.5f,
    0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,
    -0.5f, -0.5f,  0.5f,
    // left face
    -0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,
    // right face
    0.5f,  0.5f,  0.5f,
    0.5f, -0.5f, -0.5f,
    0.5f,  0.5f, -0.5f,
    0.5f, -0.5f, -0.5f,
    0.5f,  0.5f,  0.5f,
    0.5f, -0.5f,  0.5f,
    // bottom face
    -0.5f, -0.5f, -0.5f,
    0.5f, -0.5f, -0.5f,
    0.5f, -0.5f,  0.5f,
    0.5f, -0.5f,  0.5f,
    -0.5f, -0.5f,  0.5f,
    -0.5f, -0.5f, -0.5f,
    // top face
    -0.5f,  0.5f, -0.5f,
    0.5f,  0.5f,  0.5f,
    0.5f,  0.5f, -0.5f,
    0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f, -0.5f,
    -0.5f,  0.5f,  0.5,
  };

  // Cube VAO
  // --------
  unsigned int cubeVAO, cubeVBO;
  // 1. generate
  glGenVertexArrays(1, &cubeVAO);
  glGenBuffers(1, &cubeVBO);

  // 2. bind
  glBindVertexArray(cubeVAO);
  glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // 3. populate vertex data
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

  // 4. unbind
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Render the Quad to show the framebuffer
  float quadVertices[] = {
    // positions  // texCoords
    -1.0f, 1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f, 0.0f, 0.0f,
    1.0f, -1.0f,  1.0f, 0.0f,

    -1.0f, 1.0f,  0.0f, 1.0f,
    1.0f, -1.0f,  1.0f, 0.0f,
    1.0f, 1.0f,   1.0f, 1.0f,
  };
  unsigned int quadVAO, quadVBO;
  glGenVertexArrays(1, &quadVAO);
  glGenBuffers(1, &quadVBO);
  glBindVertexArray(quadVAO);
  glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

  // Framebuffer
  unsigned int FBO;
  glGenFramebuffers(1, &FBO);
  glBindFramebuffer(GL_FRAMEBUFFER, FBO);

  int width = 800;
  int height = 600;
  // generate framebuffer texture attachment
  unsigned int frameTexture;
  glGenTextures(1, &frameTexture);
  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, frameTexture);
  glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, 800, 600, GL_TRUE);
  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

  // attach a depth and stencil attachment to the framebuffer using a renderbuffer
  unsigned int RBO;
  glGenRenderbuffers(1, &RBO);
  glBindRenderbuffer(GL_RENDERBUFFER, RBO);
  glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, 800, 600);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  // Attach the texture buffer to the colour buffer
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, frameTexture, 0);
  // Attach the render buffer to the depth and stencil buffers
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

  // check if the framebuffer is complete
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // Shader for the screen quad
  Shader screenShader(
    (std::string(SHADER_DIR) + "/screen-vertex.glsl").c_str(),
    (std::string(SHADER_DIR) + "/screen-fragment.glsl").c_str()
  );
  screenShader.use();
  screenShader.setInt("framebuffer", 0);

  // Intermediate FBO that takes the blitted mutlisampled framebuffer and converts it into a texture for post processing.
  unsigned int intermediateFBO;
  glGenFramebuffers(1, &intermediateFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO);
  // create a texture colour attachment for the framebuffer
  unsigned int screenTexture;
  glGenTextures(1, &screenTexture);
  glBindTexture(GL_TEXTURE_2D, screenTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenTexture, 0);	// we only need a color buffer
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    std::cout << "ERROR::FRAMEBUFFER:: Intermediate framebuffer is not complete!" << std::endl;
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // Create a render loop that swaps the front/back buffers and polls for user events
  // Necessary to prevent the window from closing instantly
  while (!glfwWindowShouldClose(window)) {
    // delta time calculations
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // inputs
    processInput(window, deltaTime);

    // first pass - draw cube to framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // don't forget to clear the stencil buffer!
    shader.use();
    shader.setMat4("view", camera.getLookAt());
    shader.setMat4("projection", getProjection());
    shader.setMat4("model", glm::mat4(1.0));
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    // blit the multisampled buffer into an intermediate FBO
    glBindFramebuffer(GL_READ_FRAMEBUFFER, FBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, intermediateFBO);
    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    // second pass - draw framebuffer to the screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // don't forget to clear the stencil buffer!
    screenShader.use();
    glBindVertexArray(quadVAO);
    glBindTexture(GL_TEXTURE_2D, screenTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    
    // check events and swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // Deallocation
  // ------------
  glDeleteVertexArrays(1, &cubeVAO);
  glDeleteBuffers(1, &cubeVBO);
  glDeleteVertexArrays(1, &quadVAO);
  glDeleteBuffers(1, &quadVBO);

  // Terminate and clean up all resources glfwTerminate();
  glfwTerminate();
  return 0;
}

glm::mat4 getProjection() {
  return glm::perspective(glm::radians(camera.zoom), 800.0f / 600.0f, 0.1f, 100.0f);
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
