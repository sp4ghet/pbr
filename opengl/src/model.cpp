#include "model.h"

void Model::Draw(Shader &shader) {
  for (int i = 0; i < meshes.size(); ++i) {
    meshes[i].Draw(shader);
  }
}

void Model::loadModel(std::string path) {
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
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  std::vector<Texture> textures;
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

    vect.x = mesh->mBitangents[i].x;
    vect.y = mesh->mBitangents[i].y;
    vect.z = mesh->mBitangents[i].z;
    vertex.BiTangent = vect;

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
    std::vector<Texture> diffuseMaps = loadMaterialTextures(
        material, aiTextureType_DIFFUSE, "texture_diffuse");
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    std::vector<Texture> specularMaps = loadMaterialTextures(
        material, aiTextureType_SPECULAR, "texture_specular");
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    std::vector<Texture> normalMaps =
        loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
    std::vector<Texture> roughnessMaps = loadMaterialTextures(
        material, aiTextureType_DISPLACEMENT, "texture_roughness");
    textures.insert(textures.end(), roughnessMaps.begin(), roughnessMaps.end());
    printf("Found %zd diffuse, %zd specular, %zd normal, %zd roughness maps \n",
           diffuseMaps.size(), specularMaps.size(), normalMaps.size(),
           roughnessMaps.size());
  }

  return Mesh(vertices, indices, textures);
}

std::vector<Texture> Model::loadMaterialTextures(aiMaterial *mat,
                                                 aiTextureType type,
                                                 std::string texUniformName) {
  std::vector<Texture> textures;
  for (unsigned int i = 0; i < mat->GetTextureCount(type); ++i) {
    aiString str;
    mat->GetTexture(type, i, &str);
    bool skip = false;
    std::string path = std::string(str.C_Str());

    for (unsigned int j = 0; j < textures_loaded.size(); ++j) {
      if (textures_loaded.find(path) != textures_loaded.end()) {
        textures.push_back(textures_loaded[path]);
        skip = true;
        break;
      }
    }
    if (skip)
      continue;

    Texture texture;
    texture.id = TextureFromFile(str.C_Str(), directory);
    texture.uniformName = texUniformName + std::to_string(i + 1);
    textures.push_back(texture);
    texture.texType = GL_TEXTURE_2D;
    textures_loaded[path] = texture;
    printf("%s \n", texture.uniformName.c_str());
  }

  return textures;
}
