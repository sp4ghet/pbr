#pragma once

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "mesh.h"
#include "shader.h"
#include "stb/stb_image.h"
#include "textures.h"
#include <map>
#include <string>

class Model {
public:
  Model(char *path) { loadModel(path); }

  void Draw(Shader &shader);

private:
  std::vector<Mesh> meshes;
  std::map<std::string, Texture> textures_loaded;
  std::string directory;

  void loadModel(std::string path);
  void processNode(aiNode *node, const aiScene *scene);
  Mesh processMesh(aiMesh *mesh, const aiScene *scene);
  std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type,
                                            std::string typename);
};
