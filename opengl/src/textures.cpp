#include "textures.h"

unsigned int TextureFromFile(const char *path, const std::string &directory,
                             bool gamma) {
  std::string filename = std::string(path);
  filename = directory + '/' + filename;
  unsigned int textureId;
  glGenTextures(1, &textureId);

  int tw, th, nrChannel;
  unsigned char *data = stbi_load(filename.c_str(), &tw, &th, &nrChannel, 0);
  if (data) {
    GLenum format;
    if (nrChannel == 1) {
      format = GL_RED;
    }
    if (nrChannel == 2) {
      format = GL_RG;
    }
    if (nrChannel == 3) {
      format = GL_RGB;
    }
    if (nrChannel == 4) {
      format = GL_RGBA;
    }

    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexImage2D(GL_TEXTURE_2D, 0, format, tw, th, 0, format, GL_UNSIGNED_BYTE,
                 data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
  } else {
    printf("Texture failed to load at path: %s \n", path);
    stbi_image_free(data);
  }

  return textureId;
}

unsigned int loadCubemap(std::vector<std::string> faces) {
  stbi_set_flip_vertically_on_load(false);
  unsigned int textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
  int width, height, nrChannels;
  for (unsigned int i = 0; i < faces.size(); i++) {
    unsigned char *data =
        stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
    if (data) {
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height,
                   0, GL_RGB, GL_UNSIGNED_BYTE, data);
      stbi_image_free(data);
    } else {
      printf("Cubemap failed to load at path: %s\n", faces[i].c_str());
      stbi_image_free(data);
    }
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  return textureID;

  stbi_set_flip_vertically_on_load(true);
}
