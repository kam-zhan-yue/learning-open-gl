#ifndef MODEL_H
#define MODEL_H

#include "learnopengl/shader.h"
#include "learnopengl/mesh.h"
#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <assimp/types.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <stbi_image.h>

using std::vector;
using std::string;
using glm::vec3;
using glm::vec2;

class Model {
public:
  Model(const char *path) {
    loadModel(path);
    std::cout << "Loaded " << path << std::endl;
  }
  void draw(Shader &shader) {
    for (unsigned int i = 0; i < meshes.size(); ++i) {
      meshes[i].draw(shader);
    }
  }

private:
  // model data
  vector<Mesh> meshes;
  vector<Texture> loadedTextures;
  string directory;

  void loadModel(string path) {
    // 1. Declare an Importer and call its ReadFile function
    Assimp::Importer import;
    std::string resourcePath = (std::string(RESOURCES_DIR) + path);
    const aiScene *scene = import.ReadFile(resourcePath, aiProcess_Triangulate | aiProcess_FlipUVs);

    // 2. After loading the model, check if the scene and the root node are not null
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
      return;
    }

    // 3. Set the directory and process the root node, recursively
    directory = path.substr(0, path.find_last_of('/'));

    processNode(scene->mRootNode, scene);
  }

  void processNode(aiNode *node, const aiScene *scene) {
    // 1. Process all the node's meshes, if any
    for (unsigned int i=0; i<node->mNumMeshes; ++i) {
      aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
      meshes.push_back(processMesh(mesh, scene));
    }
    // 2. Do the same for its children
    for (unsigned int i=0; i<node->mNumChildren; ++i) {
      processNode(node->mChildren[i], scene);
    }
  }

  Mesh processMesh(aiMesh *mesh, const aiScene *scene) {
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;

    for (unsigned int i=0; i<mesh->mNumVertices; ++i) {
      Vertex vertex;
      // 1. Process position, normal, and texture coordinates
      vec3 position;
      position.x = mesh->mVertices[i].x;
      position.y = mesh->mVertices[i].y;
      position.z = mesh->mVertices[i].z;

      vec3 normal;
      normal.x = mesh->mNormals[i].x;
      normal.y = mesh->mNormals[i].y;
      normal.z = mesh->mNormals[i].z;

      vec2 texture = vec2(0.0);
      if (mesh->mTextureCoords[0]) { // check if the mesh contains texture coordinates
        // assimp has up to 8 different texture coordinates per vertex, but we are only going to use two
        texture.x = mesh->mTextureCoords[0][i].x;
        texture.y = mesh->mTextureCoords[0][i].y;
      }
      vertex.Position = position;
      vertex.Normal = normal;
      vertex.TexCoords = texture;
      vertices.push_back(vertex);
    }

    // 2. Process indices
    for (unsigned int i=0; i<mesh->mNumFaces; ++i) {
      aiFace face = mesh->mFaces[i];
      for (unsigned int j=0; j<face.mNumIndices; ++j) {
        indices.push_back(face.mIndices[j]);
      }
    }

    // 3. Process material
    if (mesh->mMaterialIndex >= 0) {
      aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
      // inserts all diffuseMaps into textures
      vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
      textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

      vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
      textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    }

    return Mesh(vertices, indices, textures);
  }

  vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName) {
    vector<Texture> textures;
    for (unsigned int i=0; i<mat->GetTextureCount(type); ++i) {
      aiString str;
      mat->GetTexture(type, i, &str);


      bool skip = false;
      // Compare the texture path with all the already loaded textures and skip if loaded already
      for (unsigned int j=0; j<loadedTextures.size(); ++j) {
        if (std::strcmp(loadedTextures[j].path.data(), str.C_Str()) == 0) {
          textures.push_back(loadedTextures[j]);
          skip = true;
          break;
        }
      }

      if (skip) continue;

      Texture texture;
      texture.id = loadTexture(str.C_Str(), this->directory);
      texture.type = typeName;
      texture.path = str.C_Str();
      textures.push_back(texture);
      loadedTextures.push_back(texture);
    }
    return textures;
  }

  unsigned int loadTexture(char const* path, const string& directory) {
    string filename = string(path);
    filename = directory + "/" + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);
    
    int width, height, nrChannels;
    std::string resourcePath = (std::string(RESOURCES_DIR) + filename);
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
      glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
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
};



#endif
