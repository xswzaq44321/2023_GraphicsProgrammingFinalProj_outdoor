#pragma once

#include "../../Externals/Include/assimp/Importer.hpp"
#include "../../Externals/Include/assimp/cimport.h"
#include "../../Externals/Include/assimp/scene.h"
#include "../../Externals/Include/assimp/postprocess.h"

#include <vector>
#include <map>
#include <string>

struct MyTexture {
    enum TextureType {
        DIFFUSE, SPECULAR, NORMAL
    }type;
    int width;
    int height;
    std::vector<unsigned char> data;
};

struct MyMesh
{
    std::vector<float> positions;
    std::vector<float> texcoords;
    std::vector<float> normals;
    std::vector<unsigned int> indices;
    std::vector<std::shared_ptr<MyTexture>> diffuseTexture, normalTexture;
};

class ProxyLoad {
    std::map<std::string, std::shared_ptr<MyTexture>> loadedTexs;
    std::string textureDir;
    int texcoordSize;
public:
    ProxyLoad(int texcoordSize = 2);
    std::vector<MyMesh> MyLoadObj(const std::string& filePath, const std::string& textureDir = "");
    void processNode(const aiNode *, const aiScene *, std::vector<MyMesh>&);
    MyMesh processMesh(const aiMesh *, const aiScene *);
    void processMaterial(const aiMaterial *, const aiScene *, const aiTextureType, std::vector<std::shared_ptr<MyTexture>>&);
};