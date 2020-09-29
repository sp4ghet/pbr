#pragma once

#include "glad/glad.h"
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <sstream>
#include <string>

class Shader {
public:
  unsigned int ID;
  const char *vPath;
  const char *fPath;

  Shader(const char *vertexPath, const char *fragmentPath);
  void use();

  void recompile();
  void setBool(const std::string &name, bool value) const;
  void setInt(const std::string &name, int value) const;
  void setFloat(const std::string &name, float value) const;
  void setVec3(const std::string &name, float valx, float valy,
               float valz) const;
  void setVec4(const std::string &name, float valx, float valy, float valz,
               float valw) const;

  void setMat4(const std::string &name, glm::mat4 value) const;

private:
  void loadShaders(const char *vertexPath, const char *fragmentPath);
};
