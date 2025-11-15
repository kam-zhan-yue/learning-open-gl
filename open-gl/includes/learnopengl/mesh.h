#ifndef MESH_H
#define MESH_H

#include "learnopengl/shader.h"
#include <vector>
#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

using std::vector;
using std::string;
using glm::vec3;
using glm::vec2;

struct Vertex {
  vec3 Position;
  vec3 Normal;
  vec2 TexCoords;
};

struct Texture {
  unsigned int id;
  string type;
  string path;
};

class Mesh {
public:
  vector<Vertex> vertices;
  vector<unsigned int> indices;
  vector<Texture> textures;

  Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures) {
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;

    setupMesh();
  }

  unsigned int VAO, VBO, EBO;

  void setupMesh() {
    // 1. Generate GLFW Buffers
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // 2. Populate data into the VAO and VBO
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    // 3. Populate data into the EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // 4. Set the vertex positions, normals, and texture coords
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

    glBindVertexArray(0);
  }

  void setMaterial(Shader &shader) {
    unsigned int diffuseNum = 1;
    unsigned int specularNum = 1;

    for(unsigned int i=0; i<textures.size(); ++i) {
      glActiveTexture(GL_TEXTURE0 + i);
      string number;
      string name = textures[i].type;
      if (name == "texture_diffuse") {
        number = std::to_string(diffuseNum++);
      } else if (name == "texture_specular") {
        number = std::to_string(specularNum++);
      }

      shader.setInt(("material." + name + number).c_str(), i);
      shader.setFloat("material.shininess", 32.0f);
      glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }

    glActiveTexture(GL_TEXTURE0);
  }

  void draw(Shader &shader) {
    setMaterial(shader);

    // draw mesh
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
  }
};

#endif
