#include "MagicRock.h"

MagicRock::MagicRock() {
    MyMesh mesh = ProxyLoad(3).MyLoadObj("assets/MagicRock/magicRock.obj").at(0);
    int numVertex = mesh.positions.size() / 3;
    int strideV = 9;
    my_dynamicSO = std::make_unique<DynamicSceneObject>(numVertex, mesh.indices.size(), true, true);

    float* dataBuffer = my_dynamicSO->dataBuffer();
    for (int i = 0; i < numVertex; ++i) {
        for (int j = 0; j < 3; ++j) {
            dataBuffer[i * strideV + j] = mesh.positions.at(i * 3 + j);
        }
        for (int j = 0; j < 3; ++j) {
            dataBuffer[i * strideV + 3 + j] = mesh.normals.at(i * 3 + j);
        }
        for (int j = 0; j < 3; ++j) {
            dataBuffer[i * strideV + 6 + j] = mesh.texcoords.at(i * 3 + j);
        }
    }
    my_dynamicSO->updateDataBuffer(0, numVertex * strideV * sizeof(float));

    unsigned int* indexBuffer = my_dynamicSO->indexBuffer();
    for (int i = 0; i < mesh.indices.size(); ++i) {
        indexBuffer[i] = mesh.indices.at(i);
    }
    my_dynamicSO->updateIndexBuffer(0, mesh.indices.size() * sizeof(unsigned int));

    // albedo map
    const auto& diffuseTex = mesh.diffuseTexture.at(0);
    GLuint albedoTexHandle = DynamicSceneObject::createTexture(diffuseTex->data.data(), 4, diffuseTex->width, diffuseTex->height, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GL_CLAMP_TO_EDGE, GL_NEAREST);
    my_dynamicSO->setAlbedoTex(albedoTexHandle);

    // normal map
    const auto& normalTex = mesh.normalTexture.at(0);
    GLuint normalTexHandle = DynamicSceneObject::createTexture(normalTex->data.data(), 4, normalTex->width, normalTex->height, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GL_CLAMP_TO_EDGE, GL_NEAREST);
    my_dynamicSO->setNormalTex(normalTexHandle);
    
    my_dynamicSO->setModelMat(glm::translate(glm::mat4(1.0f), glm::vec3(25.92f, 18.27f, 11.75f)));

    my_dynamicSO->setPrimitive(GL_TRIANGLES);
    my_dynamicSO->setVertexFunctionId(SceneManager::Instance()->m_vs_commonProcess);
    my_dynamicSO->setPixelFunctionId(SceneManager::Instance()->my_fs_stonePass);
}

DynamicSceneObject * MagicRock::sceneObject() const
{
    return my_dynamicSO.get();
}
