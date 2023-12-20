#include "Plane.h"

#include "MyMesh.h"

Plane::Plane(): mesh(ProxyLoad(3).MyLoadObj("assets/airplane.obj").at(0))
{
    //MyMesh mesh = ProxyLoad(3).MyLoadObj("assets/airplane.obj").at(0);
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

    my_dynamicSO->setPrimitive(GL_TRIANGLES);
    my_dynamicSO->setPixelFunctionId(SceneManager::Instance()->my_fs_planePass);

    // albedo map
    const auto& diffuseTex = mesh.diffuseTexture.at(0);
    GLuint albedoTexHandle = DynamicSceneObject::createTexture(diffuseTex->data.data(), 4, diffuseTex->width, diffuseTex->height, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GL_CLAMP_TO_EDGE, GL_NEAREST);
    my_dynamicSO->setAlbedoTex(albedoTexHandle);
}

DynamicSceneObject * Plane::sceneObject() const
{
    return my_dynamicSO.get();
}
