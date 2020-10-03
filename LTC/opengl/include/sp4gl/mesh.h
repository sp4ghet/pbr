#pragma once
#include <glad/glad.h> // holds all OpenGL type declarations

#include "shader.h"
#include "stb/stb_image.h"
#include "textures.h"
#include <glm/glm.hpp>
#include <map>
#include <string>
#include <vector>

struct Vertex {
  glm::vec3 Position;
  glm::vec3 Normal;
  glm::vec2 TexCoords;
  glm::vec3 Tangent;
  glm::vec3 BiTangent;

  Vertex(glm::vec3 pos, glm::vec3 n, glm::vec2 uv)
      : Position(pos), Normal(n), TexCoords(uv) {}
  Vertex(glm::vec3 pos, glm::vec3 n, glm::vec3 t, glm::vec2 uv)
      : Position(pos), Normal(n), TexCoords(uv), Tangent(t) {
    BiTangent = glm::normalize(glm::cross(n, t));
  }
  Vertex() {}
};

class Mesh {
public:
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  std::vector<Texture> textures;
  Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices,
       std::vector<Texture> textures) {
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;

    // now that we have all the required data, set the vertex buffers and its
    // attribute pointers.
    setupMesh();
  }

  Mesh() {}

  void Draw(Shader &shader);

private:
  unsigned int VAO, VBO, EBO;
  void setupMesh();
};
