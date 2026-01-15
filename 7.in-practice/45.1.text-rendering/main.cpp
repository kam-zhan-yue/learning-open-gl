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
#include <map>
#include <freetype/freetype.h>

using std::string;

// Data Structures
struct Character {
  unsigned int id;      // ID handle of the glyph texture
  ivec2 size;            // size of the glyph
  ivec2 bearing;         // offset from the baseline to the left/top of the glyph
  long advance; // offset to advance to the next glyph
};

struct TextRenderer {
  Shader shader;
  map<char, Character> characters;
  mat4 projection;
  unsigned int VAO;
  unsigned int VBO;
};

// Function Headers
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window, float &deltaTime);
void mouseCallback(GLFWwindow *window, double xPos, double yPos);
TextRenderer loadRenderer();
void renderText(TextRenderer renderer, string text, float x, float y, float scale, vec3 colour);

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
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Hide cursor and register cursor callback
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(window, mouseCallback);

  // NOTE(ALEX): On High DPI Displays, the logical screen size is not the same as the window screen size.
  // This ensures that we have the most accurate screen size after we've created the window
  glfwGetFramebufferSize(window, &windowWidth, &windowHeight);

  return window;
}

int main() {
  GLFWwindow *window = init();

  TextRenderer renderer = loadRenderer();

  // Create a render loop that swaps the front/back buffers and polls for user events
  // Necessary to prevent the window from closing instantly
  while (!glfwWindowShouldClose(window)) {
    // delta time calculations
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // inputs
    processInput(window, deltaTime);

    renderText(renderer, "This is sample text.", 25.0f, 25.0f, 1.0f, vec3(0.5, 0.8, 0.2));
    renderText(renderer, "LearnOpenGL", 540.0f, 570.0f, 0.5f, vec3(0.3, 0.7, 0.9));

    // check events and swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // Terminate and clean up all resources glfwTerminate();
  return 0;
}

TextRenderer loadRenderer() {
  // FreeType setup
  FT_Library ft;
  if (FT_Init_FreeType(&ft)) {
    cout << "ERROR::FREETYPE: Could not init FreeType Library" << endl;
    throw exception();
  }

  FT_Face face;
  string path = string(RESOURCES_DIR) + "/fonts/Antonio-Regular.ttf";
  if (FT_New_Face(ft, path.c_str() , 0, &face)) {
    cout << "ERROR:FREETYPE: Failed to load font" << endl;
    throw exception();
  }

  FT_Set_Pixel_Sizes(face, 0, 48);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // no byte-alignment restriction

  map<char, Character> characters;
  for (unsigned char c = 0; c < 128; ++c) {
    if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
      cout << "ERROR::FREETYPE: Failed to load glyph " << c << endl;
      continue;
    }

    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(
      GL_TEXTURE_2D, 0, GL_RED, 
      face->glyph->bitmap.width, 
      face->glyph->bitmap.rows, 
      0, GL_RED, GL_UNSIGNED_BYTE, 
      face->glyph->bitmap.buffer
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    Character character = {
      .id = texture,
      .size = ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
      .bearing = ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
      .advance = face->glyph->advance.x,
    };
    characters.emplace(c, character);
  }

  FT_Done_Face(face);
  FT_Done_FreeType(ft);

  mat4 projection = ortho(0.0f, 800.0f, 0.0f, 600.0f);

  unsigned VAO, VBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  Shader textShader = Shader(
    (string(SHADER_DIR) + "/text.vert").c_str(), 
    (string(SHADER_DIR) + "/text.frag").c_str()
  );

  return {
    .shader = textShader,
    .characters = characters,
    .projection = projection,
    .VAO = VAO,
    .VBO = VBO,
  };
}

void renderText(TextRenderer renderer, string text, float x, float y, float scale, vec3 colour) {
  renderer.shader.use();
  renderer.shader.setVec3("colour", colour);
  renderer.shader.setMat4("projection", renderer.projection);
  renderer.shader.setInt("text", 0);
  glActiveTexture(GL_TEXTURE0);
  glBindVertexArray(renderer.VAO);

  string::const_iterator c;
  for (c = text.begin(); c != text.end(); ++c) {
    Character ch = renderer.characters[*c];

    float xPos = x + ch.bearing.x * scale;
    float yPos = y - (ch.size.y - ch.bearing.y) * scale;

    float w = ch.size.x * scale;
    float h = ch.size.y * scale;

    // update VBO for each character
    float vertices[6][4] = {
      { xPos,     yPos + h,   0.0f,   0.0f  },
      { xPos,     yPos,       0.0f,   1.0f  },
      { xPos + w, yPos,       1.0f,   1.0f  },

      { xPos ,    yPos + h,   0.0f,   0.0f  },
      { xPos + w, yPos,       1.0f,   1.0f  },
      { xPos + w, yPos + h,   1.0f,   0.0f  },
    };

    if (*c != ' ') {
      glBindTexture(GL_TEXTURE_2D, ch.id); // render glyph texture over the quad
      glBindBuffer(GL_ARRAY_BUFFER, renderer.VBO); // update content of the buffer
      glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
      glDrawArrays(GL_TRIANGLES, 0, 6); // render quad
      glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    // advance cursors for next glyph (advance is 1/64 pixels)
    x += (ch.advance >> 6) * scale; // bitshift by 6 (2^6 = 64)
  }
  glBindVertexArray(0);
  glBindTexture(GL_TEXTURE_2D, 0);
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
