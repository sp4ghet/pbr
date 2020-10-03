#include "shader.h"

Shader::Shader() {}

Shader::Shader(const char *vertexPath, const char *fragmentPath)
    : vPath(vertexPath), fPath(fragmentPath) {

  loadShaders(vPath, fPath);
}

void Shader::use() { glUseProgram(ID); }

void Shader::recompile() { loadShaders(vPath, fPath); }

void Shader::setBool(const std::string &name, bool value) const {
  glUniform1ui(glGetUniformLocation(ID, name.c_str()), (int)value);
}
void Shader::setInt(const std::string &name, int value) const {
  glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}
void Shader::setFloat(const std::string &name, float value) const {
  glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}
void Shader::setVec3(const std::string &name, float valx, float valy,
                     float valz) const {
  glUniform3f(glGetUniformLocation(ID, name.c_str()), valx, valy, valz);
}

void Shader::setVec4(const std::string &name, float valx, float valy,
                     float valz, float valw) const {
  glUniform4f(glGetUniformLocation(ID, name.c_str()), valx, valy, valz, valw);
}

void Shader::setMat4(const std::string &name, glm::mat4 value) const {
  glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE,
                     glm::value_ptr(value));
}

void Shader::setVec3(const std::string &name, glm::vec3 value) const {
  glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1,
               glm::value_ptr(value));
}

void Shader::setVec3Array(const std::string &name,
                          std::vector<glm::vec3> values) const {
  for (int i = 0; i < values.size(); ++i) {
    std::string aName = (name + "[" + std::to_string(i) + "]");
    glUniform3fv(glGetUniformLocation(ID, aName.c_str()), 1,
                 glm::value_ptr(values[i]));
  }
}

// private ---------------------------------------------------------------------

void Shader::loadShaders(const char *vertexPath, const char *fragmentPath) {
  std::string vertexCode;
  std::string fragmentCode;
  std::ifstream vShaderFile;
  std::ifstream fShaderFile;

  vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

  try {
    vShaderFile.open(vertexPath);
    fShaderFile.open(fragmentPath);

    std::stringstream vShaderStream, fShaderStream;
    vShaderStream << vShaderFile.rdbuf();
    fShaderStream << fShaderFile.rdbuf();

    vShaderFile.close();
    fShaderFile.close();

    vertexCode = vShaderStream.str();
    fragmentCode = fShaderStream.str();

  } catch (std::ifstream::failure e) {
    printf("ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ\n");
    printf("%d %s\n", e.code().value(), e.what());
  }

#ifdef DEBUG
  printf("%s", vertexCode.c_str());
  printf("%s", fragmentCode.c_str());
#endif

  const char *vShaderCode = vertexCode.c_str();
  const char *fShaderCode = fragmentCode.c_str();

  int success;
  char infoLog[512];

  unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vShaderCode, NULL);
  glCompileShader(vertexShader);

  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n %s", infoLog);
  }

  unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fShaderCode, NULL);
  glCompileShader(fragmentShader);

  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
    printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n %s", infoLog);
  }

  ID = glCreateProgram();
  glAttachShader(ID, vertexShader);
  glAttachShader(ID, fragmentShader);
  glLinkProgram(ID);

  glGetProgramiv(ID, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(ID, 512, NULL, infoLog);
    printf("ERROR::SHADER::PROGRAM::LINK_FAILED\n %s", infoLog);
  }

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
}
