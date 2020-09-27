#pragma once

#include "glad.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

class Shader {
public:
  unsigned int ID;

  Shader(const char *vertexPath, const char *fragmentPath);
  void use();

  void setBool(const std::string &name, bool value) const;
  void setInt(const std::string &name, int value) const;
  void setFloat(const std::string &name, float value) const;
  void setVec3(const std::string &name, float valx, float valy,
               float valz) const;
  void setVec4(const std::string &name, float valx, float valy, float valz,
               float valw) const;
};
