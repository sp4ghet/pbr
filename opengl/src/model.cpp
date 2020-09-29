#include "model.h"

void Model::Draw(Shader &shader) {
  for (int i = 0; i < meshes.size(); ++i) {
    meshes[i].Draw(shader);
  }
}

void Model::loadModel(string path) {
  Assimp::Importer import;
  const aiScene *scene =
      import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs |
                                aiProcess_CalcTangentSpace);
  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !scene->mRootNode) {
    printf("ERROR::ASSIMP::%s\n", import.GetErrorString());
    return;
  }

  directory = path.substr(0, path.find_last_of("/"));
  processNode(scene->mRootNode, scene);
  printf("loaded %zd textures\n", textures_loaded.size());
}

void Model::processNode(aiNode *node, const aiScene *scene) {
  for (unsigned int i = 0; i < node->mNumMeshes; ++i) {

    aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
    meshes.push_back(processMesh(mesh, scene));
  }

  for (unsigned int i = 0; i < node->mNumChildren; ++i) {
    printf("Node %d/%d: %s\n", i + 1, node->mNumChildren,
           node->mChildren[i]->mName.C_Str());
    processNode(node->mChildren[i], scene);
  }
}

Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene) {
  vector<Vertex> vertices;
  vector<unsigned int> indices;
  vector<Texture> textures;
  for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
    Vertex vertex;
    glm::vec3 vect;
    vect.x = mesh->mVertices[i].x;
    vect.y = mesh->mVertices[i].y;
    vect.z = mesh->mVertices[i].z;
    vertex.Position = vect;

    vect.x = mesh->mNormals[i].x;
    vect.y = mesh->mNormals[i].y;
    vect.z = mesh->mNormals[i].z;
    vertex.Normal = vect;

    vect.x = mesh->mTangents[i].x;
    vect.y = mesh->mTangents[i].y;
    vect.z = mesh->mTangents[i].z;
    vertex.Tangent = vect;

    vertex.BiTangent = glm::cross(vertex.Normal, vertex.Tangent);

    if (mesh->mTextureCoords[0]) {
      glm::vec2 vec;
      vec.x = mesh->mTextureCoords[0][i].x;
      vec.y = mesh->mTextureCoords[0][i].y;
      vertex.TexCoords = vec;
    } else {
      vertex.TexCoords = glm::vec2(0.);
    }

    vertices.push_back(vertex);
  }

  for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
    aiFace face = mesh->mFaces[i];
    for (unsigned int j = 0; j < face.mNumIndices; j++) {
      indices.push_back(face.mIndices[j]);
    }
  }

  if (mesh->mMaterialIndex >= 0) {
    aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
    vector<Texture> diffuseMaps = loadMaterialTextures(
        material, aiTextureType_DIFFUSE, "texture_diffuse");
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    vector<Texture> specularMaps = loadMaterialTextures(
        material, aiTextureType_SPECULAR, "texture_specular");
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    vector<Texture> normalMaps =
        loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
    vector<Texture> roughnessMaps = loadMaterialTextures(
        material, aiTextureType_DISPLACEMENT, "texture_roughness");
    textures.insert(textures.end(), roughnessMaps.begin(), roughnessMaps.end());
    printf("Found %zd diffuse, %zd specular, %zd normal, %zd roughness maps \n",
           diffuseMaps.size(), specularMaps.size(), normalMaps.size(),
           roughnessMaps.size());
    for (Texture t : roughnessMaps) {
      printf("Roughness map: %s\n", t.path.c_str());
    }
  }

  return Mesh(vertices, indices, textures);
}

vector<Texture> Model::loadMaterialTextures(aiMaterial *mat, aiTextureType type,
                                            string typeName) {
  vector<Texture> textures;
  for (unsigned int i = 0; i < mat->GetTextureCount(type); ++i) {
    aiString str;
    mat->GetTexture(type, i, &str);
    bool skip = false;

    for (unsigned int j = 0; j < textures_loaded.size(); ++j) {
      if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0) {
        textures.push_back(textures_loaded[j]);
        skip = true;
        break;
      }
    }
    if (skip)
      continue;

    Texture texture;
    texture.id = TextureFromFile(str.C_Str(), directory);
    texture.type = typeName;
    texture.path = str.C_Str();
    textures.push_back(texture);
    textures_loaded.push_back(texture);
  }

  return textures;
}

unsigned int TextureFromFile(const char *path, const string &directory,
                             bool gamma) {
  string filename = string(path);
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
