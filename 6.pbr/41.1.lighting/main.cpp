#include <glm/gtc/matrix_transform.hpp>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/shapes.h>
#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <stbi_image.h>

struct Light {
  vec3 position;
  vec3 colour;
};

struct Shaders {
  Shader pbr;
};

struct Shapes {
  Cube cube;
  Sphere sphere;
};

struct Scene {
  vector<Light> lights;
  Shaders shaders;
  Shapes shapes;
};

// Function Headers
Scene generateScene();
void renderScene(Scene scene);
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
Camera camera(vec3(0.0f, 0.0f, 3.0f));
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

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_DEPTH_TEST);

  return window;
}

int main() {
  GLFWwindow *window = init();

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
    glClearColor(0.2, 0.2, 0.2, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderScene(scene);

    // check events and swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // Terminate and clean up all resources glfwTerminate();
  glfwTerminate();
  return 0;
}

Shaders generateShaders() {
  Shader pbr = Shader(
    (string(SHADER_DIR) + "/pbr-vertex.glsl").c_str(),
    (string(SHADER_DIR) + "/pbr-fragment.glsl").c_str()
  );
  return { 
    .pbr = pbr,
  };
}

float lerpFloat(float a, float b, float f) {
  return a + f * (b - a);
}

Shapes generateShapes() {
  return {
    .cube = Cube(),
    .sphere = Sphere(),
  };
}

Scene generateScene() {
  vector<Light> lights;
  lights.push_back({ vec3(-10.0f, 10.0f, 10.0f), vec3(300.0f, 300.0f, 300.0f) });
  lights.push_back({ vec3( 10.0f, 10.0f, 10.0f), vec3(300.0f, 300.0f, 300.0f) });
  lights.push_back({ vec3(-10.0f,-10.0f, 10.0f), vec3(300.0f, 300.0f, 300.0f) });
  lights.push_back({ vec3( 10.0f,-10.0f, 10.0f), vec3(300.0f, 300.0f, 300.0f) });

  return {
    .lights = lights,
    .shaders = generateShaders(),
    .shapes = generateShapes(),
  };
}

void renderScene(Scene scene) {
  Shader shader = scene.shaders.pbr;
  shader.use();
  shader.setMat4("view", camera.getLookAt());
  shader.setMat4("projection", camera.getPerspective());
  mat4 model = mat4(1.0f);

  model = mat4(1.0f);

  // render spheres
  const int ROWS = 7;
  const int COLS = 7;
  const float SPACING = 2.5;
  for (int row = 0; row < ROWS; ++row) {
    shader.setFloat("metallic", (float)row / ROWS);
    for (int col = 0; col < COLS; ++col) {
      shader.setFloat("roughness", glm::clamp((float)col / COLS, 0.05f, 1.0f));
      model = mat4(1.0f);
      model = translate(model, vec3(
        (col - (COLS / 2.0f)) * SPACING,
        (row - (ROWS / 2.0f)) * SPACING,
        0.0
      ));
      shader.setMat4("model", model);
      shader.setMat3("normalMatrix", transpose(inverse(mat3(model))));
      scene.shapes.sphere.draw();
    }
  }

  // render lights
  for (int i = 0; i < scene.lights.size(); ++i) {
    vec3 newPos = scene.lights[i].position + vec3(sin(glfwGetTime() * 5.0) * 5.0, 0.0, 0.0);
    shader.setVec3("lights[" + to_string(i) + "].position", newPos);
    shader.setVec3("lights[" + to_string(i) + "].colour", scene.lights[i].colour);

    model = glm::mat4(1.0f);
    model = translate(model, newPos);
    model = scale(model, vec3(0.5f));
    shader.setMat4("model", model);
    shader.setMat3("normalMatrix", transpose(inverse(mat3(model))));
    scene.shapes.sphere.draw();
  }
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
