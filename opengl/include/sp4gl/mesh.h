#pragma once
#include <glad/glad.h> // holds all OpenGL type declarations

#include "shader.h"
#include <glm/glm.hpp>
#include <string>
#include <vector>

using namespace std;

struct Vertex {
  glm::vec3 Position;
  glm::vec3 Normal;
  glm::vec2 TexCoords;
  glm::vec3 Tangent;
  glm::vec3 BiTangent;

  Vertex(glm::vec3 pos, glm::vec3 n, glm::vec2 uv)
      : Position(pos), Normal(n), TexCoords(uv) {}
  Vertex() {}
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
  Mesh(vector<Vertex> vertices, vector<unsigned int> indices,
       vector<Texture> textures) {
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;

    // now that we have all the required data, set the vertex buffers and its
    // attribute pointers.
    setupMesh();
  }

  void Draw(Shader &shader);

private:
  unsigned int VAO, VBO, EBO;
  void setupMesh();
};

unsigned int TextureFromFile(const char *path, const string &directory,
                             bool gamma = false);
