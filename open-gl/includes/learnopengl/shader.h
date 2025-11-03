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
	
	// constructor reads and builds the shader
  Shader(const char* vertexPath, const char* fragmentPath) {
    // 1: Retrieve the vertex / fragment source code from the filepaths
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    
    // Ensure ifstream objects can throw exceptions
    vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    
    try {
      // Open files
      vShaderFile.open(vertexPath);
      fShaderFile.open(fragmentPath);
    
      // Read files' buffer contents into stream
      std::stringstream vShaderStream, fShaderStream;
      vShaderStream << vShaderFile.rdbuf();
      fShaderStream << fShaderFile.rdbuf();
    
      // Close file handlers
      vShaderFile.close();
      fShaderFile.close();
      
      // Convert stream into string
      vertexCode = vShaderStream.str();
      fragmentCode = fShaderStream.str();
    } catch(std::ifstream::failure e) {
      std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
    }

    /*std::cout << "Vertex Path is:\n" << vertexPath << std::endl;*/
    /*std::cout << "Vertex Code is:\n" << vertexCode << std::endl;*/
    /*std::cout << "Fragment Code is:\n" << fragmentCode << std::endl;*/
    
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();
    // 2 - Compile Shaders
    unsigned int vertex, fragment;
    int success;
    char infoLog[512];

    // Vertex Shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);

    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(vertex, 512, NULL, infoLog);
      std::cout << "ERROR:SHADER::VERTEX::COMPILATION_FAILURE\n" << infoLog << std::endl;
    }

    // Fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);

    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(vertex, 512, NULL, infoLog);
      std::cout << "ERROR:SHADER::FRAGMENT::COMPILATION_FAILURE\n" << infoLog << std::endl;
    }

    // 3 - Link Shader
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);

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
};

#endif

