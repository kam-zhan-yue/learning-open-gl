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

  Scene scene = generateScene();

  glViewport(0, 0, windowWidth, windowHeight);

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
  Shader background = Shader(
    (string(SHADER_DIR) + "/background-vertex.glsl").c_str(),
    (string(SHADER_DIR) + "/background-fragment.glsl").c_str()
  );
  Shader quad = Shader(
    (string(SHADER_DIR) + "/quad-vertex.glsl").c_str(),
    (string(SHADER_DIR) + "/quad-fragment.glsl").c_str()
  );
  return { 
    .pbr = pbr,
    .background = background,
    .quad = quad,
  };
}

float lerpFloat(float a, float b, float f) {
  return a + f * (b - a);
}

Shapes generateShapes() {
  return {
    .cube = Cube(),
    .quad = Quad(),
    .sphere = Sphere(),
  };
}

Textures generateTextures() {
  unsigned int albedo = loadTexture("/textures/rusted_iron/albedo.png");
  unsigned int normal = loadTexture("/textures/rusted_iron/normal.png");
  unsigned int metallic = loadTexture("/textures/rusted_iron/metallic.png");
  unsigned int roughness = loadTexture("/textures/rusted_iron/roughness.png");
  unsigned int ao = loadTexture("/textures/rusted_iron/ao.png");

  return {
    .albedo = albedo,
    .normal = normal,
    .metallic = metallic,
    .roughness = roughness,
    .occlusion = ao,
  };
}

Environment generateEnvironment() {
  // Shader
  Shader cubemapShader = Shader(
    (string(SHADER_DIR) + "/cubemap-vertex.glsl").c_str(),
    (string(SHADER_DIR) + "/cubemap-fragment.glsl").c_str()
  );
  Shader irradianceShader = Shader(
    (string(SHADER_DIR) + "/cubemap-vertex.glsl").c_str(),
    (string(SHADER_DIR) + "/irradiance-convolution.glsl").c_str()
  );
  Shader prefilterShader = Shader(
    (string(SHADER_DIR) + "/cubemap-vertex.glsl").c_str(),
    (string(SHADER_DIR) + "/prefilter-convolution.glsl").c_str()
  );
  Shader brdfShader = Shader(
    (string(SHADER_DIR) + "/brdf-vertex.glsl").c_str(),
    (string(SHADER_DIR) + "/brdf-fragment.glsl").c_str()
  );

  // Load the HDR Environment Texture
  // --------------------------------
  stbi_set_flip_vertically_on_load(true);
  int width, height, nrComponents;
  string resourcePath = (std::string(RESOURCES_DIR) + "/textures/hdr/newport_loft.hdr");
  float *data = stbi_loadf(resourcePath.c_str(), &width, &height, &nrComponents, 0);
  unsigned int hdrTexture;
  if (data) {
    glGenTextures(1, &hdrTexture);
    glBindTexture(GL_TEXTURE_2D, hdrTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(data);
  } else {
    cout << "Failed to load HDR image at " << resourcePath << endl;
  }

  // Setup a Framebuffer to Render the Cubemap to
  // --------------------------------------------
  unsigned int captureFBO, captureRBO;
  glGenFramebuffers(1, &captureFBO);
  glGenRenderbuffers(1, &captureRBO);
  glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
  glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

  unsigned int environmentCubemap;
  glGenTextures(1, &environmentCubemap);
  glBindTexture(GL_TEXTURE_CUBE_MAP, environmentCubemap);
  for (unsigned int i = 0; i < 6; ++i) {
    // store each face with 16 bit floating point values
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // enable pre-filter mipmap sampling
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // Capture to each side of the cubemap
  // ----------------------------------
  mat4 captureProjection = perspective(radians(90.0f), 1.0f, 0.1f, 10.0f);
  mat4 captureViews[] =
  {
    lookAt(vec3(0.0f, 0.0f, 0.0f), vec3( 1.0f,  0.0f,  0.0f), vec3(0.0f, -1.0f,  0.0f)),
    lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(-1.0f,  0.0f,  0.0f), vec3(0.0f, -1.0f,  0.0f)),
    lookAt(vec3(0.0f, 0.0f, 0.0f), vec3( 0.0f,  1.0f,  0.0f), vec3(0.0f,  0.0f,  1.0f)),
    lookAt(vec3(0.0f, 0.0f, 0.0f), vec3( 0.0f, -1.0f,  0.0f), vec3(0.0f,  0.0f, -1.0f)),
    lookAt(vec3(0.0f, 0.0f, 0.0f), vec3( 0.0f,  0.0f,  1.0f), vec3(0.0f, -1.0f,  0.0f)),
    lookAt(vec3(0.0f, 0.0f, 0.0f), vec3( 0.0f,  0.0f, -1.0f), vec3(0.0f, -1.0f,  0.0f))
  };
  cubemapShader.use();
  cubemapShader.setMat4("projection", captureProjection);
  cubemapShader.setInt("equirectangularMap", 0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, hdrTexture);
  Cube cube = Cube();
  glViewport(0, 0, 512, 512);
  glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
  for (unsigned int i = 0; i < 6; ++i) {
    cubemapShader.setMat4("view", captureViews[i]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, environmentCubemap, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    cube.draw();
  }

  // Let OpenGL generate mipmaps from the first mip face (combatting dots artifact)
  glBindTexture(GL_TEXTURE_CUBE_MAP, environmentCubemap);
  glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

  // Pre-compute the irradiance diffuse map
  // --------------------------------------
  unsigned int irradianceMap;
  glGenTextures(1, &irradianceMap);
  glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
  for (unsigned int i = 0; i < 6; ++i) {
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  irradianceShader.use();
  irradianceShader.setInt("environmentMap", 0);
  irradianceShader.setMat4("projection", captureProjection);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, environmentCubemap);
  glViewport(0, 0, 32, 32);
  glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
  glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);
  for (unsigned int i = 0; i < 6; ++i) {
    irradianceShader.setMat4("view", captureViews[i]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    cube.draw();
  }

  // Pre-compute the prefilter map
  // --------------------------------------
  unsigned int prefilterMap;
  glGenTextures(1, &prefilterMap);
  glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
  for (unsigned int i = 0; i < 6; ++i) {
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

  prefilterShader.use();
  prefilterShader.setInt("environmentMap", 0);
  prefilterShader.setMat4("projection", captureProjection);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, environmentCubemap);

  glBindFramebuffer(GL_FRAMEBUFFER, captureFBO); // reuse the capture fbo
  unsigned int maxMipLevels = 5;
  for (unsigned int mip = 0; mip < maxMipLevels; ++mip) {
    // resize the framebuffer according to the mip-level size
    unsigned int mipWidth = 128 * pow(0.5, mip);
    unsigned int mipHeight = 128 * pow(0.5, mip);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
    glViewport(0, 0, mipWidth, mipHeight);

    // render the prefilter convolution to each side of the cube at varying roughnesses at different mipmap levels
    float roughness = (float)mip / (maxMipLevels - 1);
    prefilterShader.setFloat("roughness", roughness);
    for (unsigned int i = 0; i < 6; ++i) {
      prefilterShader.setMat4("view", captureViews[i]);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap, mip);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      cube.draw();
    }
  }

  // Pre-compute the BRDF Map
  // --------------------------------------
  unsigned int brdfLUTTexture;
  glGenTextures(1, &brdfLUTTexture);

  // pre-allocate enough memory for the LUT texture
  glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // run the shader over an NDC screen-space quad
  glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
  glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);
  glViewport(0, 0, 512, 512);
  brdfShader.use();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  Quad quad = Quad();
  quad.draw();

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    cout << "Framebuffer is not complete." << endl;

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
  glBindTexture(GL_TEXTURE_2D, 0);

  return {
    .texture = environmentCubemap,
    .irradianceMap = irradianceMap,
    .prefilterMap = prefilterMap,
    .brdfLUTTexture = brdfLUTTexture,
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
    .textures = generateTextures(),
    .environment = generateEnvironment(),
  };
}

void renderSpheres(Scene scene) {
  Shader shader = scene.shaders.pbr;
  shader.use();
  shader.setFloat("ambientOcclusion", 1.0f);
  shader.setVec3("albedo", vec3(0.5f, 0.0f, 0.0f));
  shader.setInt("irradianceMap", 0);
  shader.setInt("prefilterMap", 1);
  shader.setInt("brdfLUT", 2);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, scene.environment.irradianceMap);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_CUBE_MAP, scene.environment.prefilterMap);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, scene.environment.brdfLUTTexture);

  shader.setVec3("camPos", camera.cameraPos);
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

void renderEnvironment(Scene scene) {
  Shader shader = scene.shaders.background;
  shader.use();
  shader.setMat4("view", camera.getLookAt());
  shader.setMat4("projection", camera.getPerspective());
  shader.setInt("environmentCubemap", 0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, scene.environment.texture);
  scene.shapes.cube.draw();
  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);


  Shader quad = scene.shaders.quad;
  quad.use();
  quad.setInt("brdf", 0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, scene.environment.brdfLUTTexture);
  scene.shapes.quad.draw();
  glBindTexture(GL_TEXTURE_2D, 0);
}

void renderScene(Scene scene) {
  renderSpheres(scene);
  renderEnvironment(scene);
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
