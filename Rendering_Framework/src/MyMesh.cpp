#include "MyMesh.h"

#include "../../Externals/Include/assimp/cimport.h"
#include "../../Externals/Include/assimp/scene.h"
#include "../../Externals/Include/assimp/postprocess.h"
#include "../../Externals/Include/stb_image.h"

ProxyLoad::ProxyLoad(int texcoordSize) :
    texcoordSize(texcoordSize)
{
}

std::vector<MyMesh> ProxyLoad::MyLoadObj(const std::string & filePath, const std::string& textureDir)
{
    Assimp::Importer importer;
    const aiScene* sceneObjPtr = importer.ReadFile(filePath, aiProcess_Triangulate | aiProcess_GenSmoothNormals);
    if (!sceneObjPtr) {
        fprintf(stderr, "Error: Model::loadModel, description: %s\n", importer.GetErrorString());
    }
    if (textureDir.empty()) {
        this->textureDir = filePath.substr(0, filePath.find_last_of('/') + 1);
    }
    else {
        this->textureDir = textureDir;
    }
    std::vector<MyMesh> rtv;
    processNode(sceneObjPtr->mRootNode, sceneObjPtr, rtv);
    return rtv;
}

void ProxyLoad::processNode(const aiNode * node, const aiScene * scene, std::vector<MyMesh>& meshes)
{
    if (!node || !scene)
        return;
    for (int i = 0; i < node->mNumMeshes; ++i) {
        const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        if (!mesh)
            continue;
        MyMesh meshObj = processMesh(mesh, scene);
        meshes.push_back(std::move(meshObj));
    }
    for (int i = 0; i < node->mNumChildren; ++i) {
        processNode(node->mChildren[i], scene, meshes);
    }
}

MyMesh ProxyLoad::processMesh(const aiMesh * mesh, const aiScene * scene)
{
    MyMesh rtv;
    for (int i = 0; i < mesh->mNumVertices; ++i) {
        if (mesh->HasPositions()) {
            rtv.positions.push_back(mesh->mVertices[i].x);
            rtv.positions.push_back(mesh->mVertices[i].y);
            rtv.positions.push_back(mesh->mVertices[i].z);
        }
        if (mesh->HasTextureCoords(0)) {
            rtv.texcoords.push_back(mesh->mTextureCoords[0][i].x);
            rtv.texcoords.push_back(mesh->mTextureCoords[0][i].y);
            if (texcoordSize == 3)
                rtv.texcoords.push_back(0);
        }
        else {
            rtv.texcoords.insert(rtv.texcoords.end(), texcoordSize, 0);
        }
        if (mesh->HasNormals()) {
            rtv.normals.push_back(mesh->mNormals[i].x);
            rtv.normals.push_back(mesh->mNormals[i].y);
            rtv.normals.push_back(mesh->mNormals[i].z);
        }
    }
    for (int i = 0; i < mesh->mNumFaces; ++i) {
        aiFace& face = mesh->mFaces[i];
        if (face.mNumIndices != 3) {
            fprintf(stderr, "Error: Model::processMesh, mesh not transformed to triangle mesh.\n");
        }
        rtv.indices.push_back(face.mIndices[0]);
        rtv.indices.push_back(face.mIndices[1]);
        rtv.indices.push_back(face.mIndices[2]);
    }
    if (mesh->mMaterialIndex >= 0) {
        const aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        processMaterial(material, scene, aiTextureType_DIFFUSE, rtv.diffuseTexture);
        processMaterial(material, scene, aiTextureType_SPECULAR, rtv.diffuseTexture);
        processMaterial(material, scene, aiTextureType_NORMALS, rtv.normalTexture);
    }
    return rtv;
}

void ProxyLoad::processMaterial(const aiMaterial* mat, const aiScene* scene, const aiTextureType textureType, std::vector<std::shared_ptr<MyTexture>>& textures)
{
    for (int i = 0; i < mat->GetTextureCount(textureType); ++i) {
        aiString textPath;
        aiReturn retStat = mat->GetTexture(textureType, i, &textPath);
        if (retStat != aiReturn_SUCCESS || textPath.length == 0) {
            fprintf(stderr, "Error: load texture type \"%d\" , index= %d failed with return %d\n", textureType, i, retStat);
            continue;
        }

        std::string path = textureDir + textPath.C_Str();
        if (loadedTexs.count(path)) {
            textures.push_back(loadedTexs[path]);
            continue;
        }

        auto text = std::make_shared<MyTexture>();
        int n;
        stbi_set_flip_vertically_on_load(true);
        stbi_uc *data = stbi_load(path.c_str(), &text->width, &text->height, &n, 4);
        if (data) {
            text->data.resize(text->width * text->height * 4 * sizeof(unsigned char));
            std::copy(data, data + text->width * text->height * 4 * sizeof(unsigned char), text->data.begin());
            stbi_image_free(data);
        }
        else {
            fprintf(stderr, "file \"%s\" not found!\n", path.c_str());
        }
        textures.push_back(text);
        loadedTexs[path] = text;
    }
}