#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader {
public:
	// the program ID
	unsigned int ID;
	
  Shader(const char* vertexPath, const char* geometryPath, const char* fragmentPath) {
    unsigned int vertex = generateShader(vertexPath, GL_VERTEX_SHADER);
    unsigned int geometry = generateShader(geometryPath, GL_GEOMETRY_SHADER);
    unsigned int fragment = generateShader(fragmentPath, GL_FRAGMENT_SHADER);

    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, geometry);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);

    int success;
    char infoLog[512];
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(ID, 512, NULL, infoLog);
      std::cout << "ERROR:SHADER::PROGRAM::LINKING_FAILURE\n" << infoLog << std::endl;
    }
    glDeleteShader(vertex);
    glDeleteShader(geometry);
    glDeleteShader(fragment);
  }

	// constructor reads and builds the shader
  Shader(const char* vertexPath, const char* fragmentPath) {
    unsigned int vertex = generateShader(vertexPath, GL_VERTEX_SHADER);
    unsigned int fragment = generateShader(fragmentPath, GL_FRAGMENT_SHADER);

    // 3 - Link Shader
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);

    int success;
    char infoLog[512];
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(ID, 512, NULL, infoLog);
      std::cout << "ERROR:SHADER::PROGRAM::LINKING_FAILURE\n" << infoLog << std::endl;
    }

    // Delete shaders as they're linked to the program and are no longer necessary
    glDeleteShader(vertex);
    glDeleteShader(fragment);
  }

  // to activate the shader
  void use() {
    glUseProgram(ID);
  }

  // utility functions
  void setBool(const std::string &name, bool value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
  }

  void setInt(const std::string &name, int value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
  }

  void setFloat(const std::string &name, float value) const {
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
  };

  void setVec3(const std::string &name, float x, float y, float z) const {
    glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
  };

  void setVec3(const std::string &name, glm::vec3 value) const {
    glUniform3f(glGetUniformLocation(ID, name.c_str()), value.x, value.y, value.z);
  };

  void setMat4(const std::string &name, glm::mat4 value) const {
    unsigned int loc = glGetUniformLocation(ID, name.c_str());
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(value));
  };

private:
  unsigned int generateShader(const char *path, unsigned int SHADER_TYPE) {
    // 1: Retrieve the vertex / fragment source code from the filepaths
    std::string shaderCode;
    std::ifstream shaderFile;
    
    // Ensure ifstream objects can throw exceptions
    shaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    
    try {
      // Open files
      shaderFile.open(path);
    
      // Read files' buffer contents into stream
      std::stringstream shaderStream;
      shaderStream << shaderFile.rdbuf();
    
      // Close file handlers
      shaderFile.close();
      
      // Convert stream into string
      shaderCode = shaderStream.str();
    } catch(std::ifstream::failure e) {
      std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
    }

    const char* code = shaderCode.c_str();
    // 2 - Compile Shaders
    unsigned int shader;
    int success;
    char infoLog[512];

    // Vertex Shader
    shader = glCreateShader(SHADER_TYPE);
    glShaderSource(shader, 1, &code, NULL);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(shader, 512, NULL, infoLog);
      std::string shaderTypeStr;
      switch (SHADER_TYPE) {
        case GL_VERTEX_SHADER:
          shaderTypeStr = "VERTEX";
          break;
        case GL_FRAGMENT_SHADER:
          shaderTypeStr = "FRAGMENT";
          break;
        case GL_GEOMETRY_SHADER:
          shaderTypeStr = "GEOMETRY";
          break;
        default:
          shaderTypeStr = "UNKNOWN";
          break;
      }
      std::cout << "ERROR:SHADER::VERTEX::COMPILATION_FAILURE\n" << infoLog << std::endl;
    }

    return shader;
    return 0;
  }
};

#endif

