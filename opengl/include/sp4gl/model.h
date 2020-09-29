#pragma once

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "mesh.h"
#include "shader.h"
#include "stb/stb_image.h"
#include <string>

class Model {
public:
  Model(char *path) { loadModel(path); }

  void Draw(Shader &shader);

private:
  vector<Mesh> meshes;
  vector<Texture> textures_loaded;
  string directory;

  void loadModel(string path);
  void processNode(aiNode *node, const aiScene *scene);
  Mesh processMesh(aiMesh *mesh, const aiScene *scene);
  vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type,
                                       string typename);
};
