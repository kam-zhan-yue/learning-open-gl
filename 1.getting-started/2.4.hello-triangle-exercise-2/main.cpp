#include <array>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

const char *vertexShaderSource = "#version 330 core\n"
       "layout (location = 0) in vec3 aPos;\n"
       "void main()\n"
       "{\n"
       " gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
       "}\0";

const char *fragmentShaderSource = "#version 330 core \n"
  "out vec4 FragColor;\n"
  "\n"
  "void main() {\n"
  "  FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
  "}\0";

// Function Headers
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
unsigned int setupShaders();
std::array<unsigned int, 2> setupVAO();
void renderTriangle(unsigned int shaderProgram, std::array<unsigned int, 2> VAO);

int main() {
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
    return -1;
  }
  glfwMakeContextCurrent(window);

  // Initialise GLAD
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialise GLAD" << std::endl;
    return -1;
  }

  // Tell OpenGL the size of the rendering window so that OpenGL knows how we want to display the data and coordinates
  glViewport(0, 0, 800, 600);


  // Register the frame buffer size callback when the user resizes the window
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  unsigned int shaderProgram = setupShaders();

  std::array<unsigned int, 2> VAO = setupVAO();

  // Create a render loop that swaps the front/back buffers and polls for user events
  // Necessary to prevent the window from closing instantly
  while (!glfwWindowShouldClose(window)) {
    // inputs
    processInput(window);

    // rendering commands
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    renderTriangle(shaderProgram, VAO);

    // check events and swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // Terminate and clean up all resources glfwTerminate();
  return 0;
}

unsigned int setupShaders() {
  // Shader Compilation
  unsigned int vertexShader;
  vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
  glCompileShader(vertexShader);

  unsigned int fragmentShader;
  fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
  glCompileShader(fragmentShader);

  // Check Shader Compilation
  unsigned int shaders[2] = {vertexShader, fragmentShader};
  for (unsigned int shader : shaders) {
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) { 
      glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
      std::cout << "ERROR:SHADER::VERTEX::COMPILATION_FAILURE\n" << infoLog << std::endl;
      return -1;
    }
  }

  // Program Object
  unsigned int shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);

  int success;
  char infoLog[512];
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
      std::cout << "ERROR:PROGRAM_LINNKING\n" << infoLog << std::endl;
      return -1;
  }

  // Delete the shaders after use
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  return shaderProgram;
}

std::array<unsigned int, 2> setupVAO() {
  // first triangle
  float vertices_1[] = {
    -0.9f, 0.9f, 0.0f,
    -0.1f, 0.9f, 0.0f,
    -0.9f, -0.9f, 0.0f,
  };
  // second triangle
  float vertices_2[] = {
    0.9f, 0.9f, 0.0f,
    0.1f, -0.9f, 0.0f,
    0.9f, -0.9f, 0.0f,
  };

  // Create two VAOs and VBOs
  std::array<unsigned int, 2> VAO{};
  unsigned int VBO[2];

  // Generate multiple VAOs and VBOs at a time
  glGenVertexArrays(2, VAO.data());
  glGenBuffers(2, VBO);

  // First Triangle Setup
  glBindVertexArray(VAO[0]);
  glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_1), vertices_1, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  // Second Triangle Setup
  glBindVertexArray(VAO[1]);
  glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_2), vertices_2, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  // unbind everything
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  return VAO;
}

void renderTriangle(unsigned int shaderProgram, std::array<unsigned int, 2> VAO) {
  for (unsigned int vao : VAO) {
    glUseProgram(shaderProgram);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
  }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
  glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }
}

