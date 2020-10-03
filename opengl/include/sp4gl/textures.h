#pragma once

#include "glad/glad.h"
#include "stb/stb_image.h"
#include <string>
#include <vector>

struct Texture {
  unsigned int id;
  unsigned int texType;
  std::string uniformName;

  Texture(unsigned int id, unsigned int texType, std::string uniformName)
      : id(id), texType(texType), uniformName(uniformName) {}
  Texture() {}
};

unsigned int TextureFromFile(const char *path, const std::string &directory,
                             bool gamma = false);
unsigned int loadCubemap(std::vector<std::string> textures_faces);
