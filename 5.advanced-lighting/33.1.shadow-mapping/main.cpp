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

using std::string;

struct Scene {
  unsigned int planeVAO;
  unsigned int cubeVAO;
  unsigned int quadVAO;
};

// Function Headers
Scene generateScene();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window, float &deltaTime);
unsigned int loadTexture(char const *path);
void mouseCallback(GLFWwindow *window, double xPos, double yPos);
void scrollCallback(GLFWwindow *window, double xPos, double yPos);
void renderScene(Scene &scene, Shader &shader);
void renderQuad(Scene &scene, Shader &shader);

// Global Variables
bool firstMouse = false;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float lastX = 400.0f;
float lastY = 300.0f;
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

bool blinn = false;
bool blinnKeyPressed = false;

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
  GLFWwindow *window = init();

  // Configue Global State
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Build Shaders
  Shader depthShader(
    (string(SHADER_DIR) + "/depth-vertex.glsl").c_str(), 
    (string(SHADER_DIR) + "/depth-fragment.glsl").c_str()
  );
  Shader shader = Shader(
    (string(SHADER_DIR) + "/vertex.glsl").c_str(),
    (string(SHADER_DIR) + "/fragment.glsl").c_str()
  );
  Shader screenShader(
    (string(SHADER_DIR) + "/quad-vertex.glsl").c_str(), 
    (string(SHADER_DIR) + "/quad-fragment.glsl").c_str()
  );

  unsigned int woodTexture = loadTexture("/textures/wood.png");

  vec3 lightPos(-2.0f, 4.0f, -1.0f);

  shader.use();
  shader.setFloat("texture1", 0);

  screenShader.use();
  screenShader.setInt("screenTexture", 0);

  // Generate a depth buffer texture
  unsigned int depthMap;
  const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
  glGenTextures(1, &depthMap);
  glBindTexture(GL_TEXTURE_2D, depthMap);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glBindTexture(GL_TEXTURE_2D, 0);

  // Generate the framebuffer object for the depth map and bind the generated texture
  unsigned int depthMapFBO;
  glGenFramebuffers(1, &depthMapFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);

  // check if the framebuffer is complete
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  const unsigned int SCREEN_WIDTH = 800, SCREEN_HEIGHT = 600;

  // Shader Configuration
  shader.use();
  shader.setInt("texture1", 0);
  shader.setInt("shadowMap", 1);
  screenShader.use();
  screenShader.setInt("screenTexture", 0);

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

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    // First Pass - Render the Depth Buffer to the framebuffer
    
    // Setup the light shader here
    float nearPlane = 1.0f, farPlane = 7.5f;
    glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, nearPlane, farPlane);
    glm::mat4 lightView = glm::lookAt(lightPos, vec3(0.0f), vec3(0.0, 1.0, 0.0));
    glm::mat4 lightSpaceMatrix = lightProjection * lightView;
    depthShader.use();
    depthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

    // Render the depth map
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, woodTexture);
    renderScene(scene, depthShader);

    // Second Pass - Render the Framebuffer on a Quad
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    renderQuad(scene, screenShader);

    // This works.
    /*shader.setMat4("view", camera.getLookAt());*/
    /*shader.setMat4("projection", camera.getPerspective());*/
    /*shader.setVec3("lightPos", lightPos);*/
    /*shader.setVec3("viewPos", camera.cameraPos);*/
    /**/
    /*glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);*/
    /*glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);*/
    /*glBindFramebuffer(GL_FRAMEBUFFER, 0);*/
    /*glActiveTexture(GL_TEXTURE0);*/
    /*glBindTexture(GL_TEXTURE_2D, woodTexture);*/
    /*renderScene(scene, shader);*/

    // check events and swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // Terminate and clean up all resources glfwTerminate();
  return 0;
}

unsigned int generatePlane() {
  float planeVertices[] = {
    // positions            // normals         // texcoords
     25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
    -25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
    -25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,

     25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
    -25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
     25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 25.0f
  };

  unsigned int planeVAO, planeVBO;
  glGenVertexArrays(1, &planeVAO);
  glGenBuffers(1, &planeVBO);
  glBindVertexArray(planeVAO);
  glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  return planeVAO;
}

unsigned int generateCube() {
  unsigned int cubeVAO, cubeVBO;

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

  glGenVertexArrays(1, &cubeVAO);
  glGenBuffers(1, &cubeVBO);
  glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  glBindVertexArray(cubeVAO);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  return cubeVAO;
}

unsigned int generateQuad() {
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
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  return quadVAO;
}

Scene generateScene() {
  unsigned int planeVAO = generatePlane();
  unsigned int cubeVAO = generateCube();
  unsigned int quadVAO = generateQuad();
  Scene scene = {
    planeVAO,
    cubeVAO,
    quadVAO,
  };
  return scene;
}

void renderCube(Scene &scene) {
  glBindVertexArray(scene.cubeVAO);
  glDrawArrays(GL_TRIANGLES, 0, 6);
}

void renderScene(Scene &scene, Shader &shader) {
  // plane
  glm::mat4 model = glm::mat4(1.0);
  shader.use();
  shader.setMat4("model", model);
  glBindVertexArray(scene.planeVAO);
  glDrawArrays(GL_TRIANGLES, 0, 6);

  // cubes
  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0));
  model = glm::scale(model, glm::vec3(0.5f));
  shader.setMat4("model", model);
  renderCube(scene);
  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(2.0f, 0.0f, 1.0));
  model = glm::scale(model, glm::vec3(0.5f));
  shader.setMat4("model", model);
  renderCube(scene);
  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 2.0));
  model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
  model = glm::scale(model, glm::vec3(0.25));
  shader.setMat4("model", model);
  renderCube(scene);

}

void renderQuad(Scene &scene, Shader &shader) {
  shader.use();
  glBindVertexArray(scene.quadVAO);
  glDrawArrays(GL_TRIANGLES, 0, 6);
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

  if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && !blinnKeyPressed) {
    blinn = !blinn;
    blinnKeyPressed = true;
  } else if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE) {
    blinnKeyPressed = false;
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
