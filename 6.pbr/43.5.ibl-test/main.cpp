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
  Shader background;
  Shader quad;
};

struct Shapes {
  Cube cube;
  Quad quad;
  Sphere sphere;
};

struct Textures {
  unsigned int albedo;
  unsigned int normal;
  unsigned int metallic;
  unsigned int roughness;
  unsigned int occlusion;
  unsigned int hdrTexture;
};

struct Environment {
  unsigned int texture;
  unsigned int irradianceMap;
  unsigned int prefilterMap;
  unsigned int brdfLUTTexture;
};

struct Scene {
  vector<Light> lights;
  Shaders shaders;
  Shapes shapes;
  Textures textures;
  Environment environment;
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
  glDepthFunc(GL_LEQUAL);
  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

  return window;
}

int main() {
  GLFWwindow *window = init();

  glViewport(0, 0, windowWidth, windowHeight);

  Quad quad = Quad();
  Shader brdf = Shader(
    (string(SHADER_DIR) + "/brdf-vertex.glsl").c_str(),
    (string(SHADER_DIR) + "/brdf-fragment.glsl").c_str()
  );

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
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    brdf.use();
    quad.draw();

    // check events and swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // Terminate and clean up all resources glfwTerminate();
  glfwTerminate();
  return 0;
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
