#include "MyIndirectRenderer.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glad/glad.h>
#include <numeric>
#include "MyMesh.h"

MyIndirectRenderer::MyIndirectRenderer()
{
    std::vector<MyMesh> meshes{
        ProxyLoad().MyLoadObj("assets/grassB.obj")[0],  // grassB
        ProxyLoad().MyLoadObj("assets/bush01_lod2.obj")[0], // bush01
        ProxyLoad().MyLoadObj("assets/bush05_lod2.obj")[0], // bush05
        ProxyLoad().MyLoadObj("assets/Medieval_Building_LowPoly/medieval_building_lowpoly_2.obj")[0],   // buildingV2
        ProxyLoad().MyLoadObj("assets/Medieval_Building_LowPoly/medieval_building_lowpoly_1.obj")[0],   // buildingV1
    };
    std::vector<std::shared_ptr<MyPoissonSample>> samples{
        std::shared_ptr<MyPoissonSample>(MyPoissonSample::fromFile("assets/poissonPoints_621043_after.ppd2")),  // grassB
        std::shared_ptr<MyPoissonSample>(MyPoissonSample::fromFile("assets/poissonPoints_1010.ppd2")),  // bush01
        std::shared_ptr<MyPoissonSample>(MyPoissonSample::fromFile("assets/poissonPoints_2797.ppd2")),  // bush05
        std::shared_ptr<MyPoissonSample>(MyPoissonSample::fromFile("assets/cityLots_sub_0.ppd2")), // buildingV2
        std::shared_ptr<MyPoissonSample>(MyPoissonSample::fromFile("assets/cityLots_sub_1.ppd2")),  // buildingV1
    };
    const int NUM_TEXTURE = meshes.size();
    const int IMG_WIDTH = 1024;
    const int IMG_HEIGHT = 1024;
    const int IMG_CHANNEL = 4;
    int numVertex = std::accumulate(meshes.begin(), meshes.end(), 0, [](int val, const MyMesh& mesh) {
        return val + mesh.positions.size();
    });
    int numIndex = std::accumulate(meshes.begin(), meshes.end(), 0, [](int val, const MyMesh& mesh) {
        return val + mesh.indices.size();
    });
    int strideV = 9;
    std::vector<float> positions;
    std::vector<float> normals;
    std::vector<float> texcoords;
    std::vector<unsigned int> indices;
    std::vector<unsigned char> texture;
    std::vector<InstanceProperties> offsets;
    this->meshCount = meshes.size();

    for (int i = 0; i < this->meshCount; ++i) {
        const MyMesh& mesh = meshes[i];
        positions.insert(positions.end(), mesh.positions.begin(), mesh.positions.end());
        normals.insert(normals.end(), mesh.normals.begin(), mesh.normals.end());
        for (int j = 0; j < mesh.texcoords.size(); j += 2) {
            texcoords.push_back(mesh.texcoords[j]);
            texcoords.push_back(mesh.texcoords[j + 1]);
            texcoords.push_back(i);
        }
        indices.insert(indices.end(), mesh.indices.begin(), mesh.indices.end());
        texture.insert(texture.end(), mesh.diffuseTexture.at(0)->data.begin(), mesh.diffuseTexture.at(0)->data.end());
    }
    for(const auto& samp : samples){
        for (int i = 0; i < samp->m_numSample; ++i) {
            InstanceProperties temp;
            temp.position.x = samp->m_positions[i * 3 + 0];
            temp.position.y = samp->m_positions[i * 3 + 1];
            temp.position.z = samp->m_positions[i * 3 + 2];
            temp.position.w = 0;
            glm::vec3 rad(samp->m_radians[i * 3 + 0], samp->m_radians[i * 3 + 1], samp->m_radians[i * 3 + 2]);
            glm::quat q = glm::quat(rad);
            temp.rotation = glm::toMat4(q);
            offsets.push_back(std::move(temp));
        }
        offsetCnt += samp->m_numSample;
    }
    offsets.shrink_to_fit();

    for (int i = 0, firstIndex = 0, baseVertex = 0, baseInstance = 0; i < this->meshCount; ++i) {
        cmds.push_back({});
        cmds[i].count = meshes[i].indices.size();
        cmds[i].instanceCount = samples[i]->m_numSample;
        cmds[i].firstIndex = firstIndex;
        cmds[i].baseVertex = baseVertex;
        cmds[i].baseInstance = baseInstance;
        firstIndex += cmds[i].count;
        baseVertex += meshes[i].positions.size() / 3;
        baseInstance += cmds[i].instanceCount;
    }

    unsigned int vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * positions.size(), positions.data(), GL_STATIC_DRAW);

    unsigned int ibo;
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), indices.data(), GL_STATIC_DRAW);

    unsigned int offsetHandle;
    glGenBuffers(1, &offsetHandle);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, offsetHandle);
    glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(InstanceProperties) * offsets.size(), offsets.data(), GL_MAP_READ_BIT);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SceneManager::Instance()->my_instanceBind, offsetHandle);

    unsigned int visibleBufferHandle = 0;
    glGenBuffers(1, &visibleBufferHandle);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, visibleBufferHandle);
    glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned int) * offsets.size(), nullptr, GL_MAP_READ_BIT);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SceneManager::Instance()->my_validBind, visibleBufferHandle);

    unsigned int cmdBufferHandleSSBO;
    glGenBuffers(1, &cmdBufferHandleSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, cmdBufferHandleSSBO);
    glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(DrawElementsIndirectCommand) * cmds.size(), cmds.data(), GL_MAP_READ_BIT);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SceneManager::Instance()->my_drawCmdBind, cmdBufferHandleSSBO);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, cmdBufferHandleSSBO);
    glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(DrawElementsIndirectCommand) * cmds.size(), cmds.data(), GL_DYNAMIC_DRAW);

    unsigned int texcoordHandle;
    glGenBuffers(1, &texcoordHandle);
    glBindBuffer(GL_ARRAY_BUFFER, texcoordHandle);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * texcoords.size(), texcoords.data(), GL_STATIC_DRAW);

    unsigned int normalHandle;
    glGenBuffers(1, &normalHandle);
    glBindBuffer(GL_ARRAY_BUFFER, normalHandle);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * normals.size(), normals.data(), GL_STATIC_DRAW);

    glGenTextures(1, &textureHandle);
    glBindTexture(GL_TEXTURE_2D_ARRAY, textureHandle);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 11, GL_RGBA8, IMG_WIDTH, IMG_HEIGHT, NUM_TEXTURE);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, IMG_WIDTH, IMG_HEIGHT, NUM_TEXTURE, GL_RGBA, GL_UNSIGNED_BYTE, texture.data());
    glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    glActiveTexture(SceneManager::Instance()->my_albedoTexArrUnit);
    glBindTexture(GL_TEXTURE_2D_ARRAY, textureHandle);

    // creating vao
    glCreateVertexArrays(1, &this->vao);
    glBindVertexArray(this->vao);

    glEnableVertexAttribArray(SceneManager::Instance()->m_vertexHandle);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(SceneManager::Instance()->m_vertexHandle, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

    //glBindBuffer(GL_ARRAY_BUFFER, offsetHandle);
    //glEnableVertexAttribArray(SceneManager::Instance()->my_offsetHandle);
    //glVertexAttribDivisor(SceneManager::Instance()->my_offsetHandle, 1);
    //glVertexAttribPointer(SceneManager::Instance()->my_offsetHandle, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceProperties), 0);
    //glEnableVertexAttribArray(SceneManager::Instance()->my_rotationHandle + 0);
    //glEnableVertexAttribArray(SceneManager::Instance()->my_rotationHandle + 1);
    //glEnableVertexAttribArray(SceneManager::Instance()->my_rotationHandle + 2);
    //glEnableVertexAttribArray(SceneManager::Instance()->my_rotationHandle + 3);
    //glVertexAttribDivisor(SceneManager::Instance()->my_rotationHandle + 0, 1);
    //glVertexAttribDivisor(SceneManager::Instance()->my_rotationHandle + 1, 1);
    //glVertexAttribDivisor(SceneManager::Instance()->my_rotationHandle + 2, 1);
    //glVertexAttribDivisor(SceneManager::Instance()->my_rotationHandle + 3, 1);
    //glVertexAttribPointer(SceneManager::Instance()->my_rotationHandle + 0, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceProperties), (void*)(offsetof(InstanceProperties, rotation) + 0));
    //glVertexAttribPointer(SceneManager::Instance()->my_rotationHandle + 1, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceProperties), (void*)(offsetof(InstanceProperties, rotation) + 16));
    //glVertexAttribPointer(SceneManager::Instance()->my_rotationHandle + 2, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceProperties), (void*)(offsetof(InstanceProperties, rotation) + 32));
    //glVertexAttribPointer(SceneManager::Instance()->my_rotationHandle + 3, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceProperties), (void*)(offsetof(InstanceProperties, rotation) + 48));

    glBindBuffer(GL_ARRAY_BUFFER, visibleBufferHandle);
    glEnableVertexAttribArray(SceneManager::Instance()->my_offsetHandle);
    glVertexAttribDivisor(SceneManager::Instance()->my_offsetHandle, 1);
    glVertexAttribIPointer(SceneManager::Instance()->my_offsetHandle, 1, GL_UNSIGNED_INT, 0, 0);

    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, cmdBufferHandleSSBO);

    glEnableVertexAttribArray(SceneManager::Instance()->m_uvHandle);
    glBindBuffer(GL_ARRAY_BUFFER, texcoordHandle);
    glVertexAttribPointer(SceneManager::Instance()->m_uvHandle, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(SceneManager::Instance()->m_normalHandle);
    glBindBuffer(GL_ARRAY_BUFFER, normalHandle);
    glVertexAttribPointer(SceneManager::Instance()->m_normalHandle, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindVertexArray(0);

    this->m_vertexFunctionId = SceneManager::Instance()->my_vs_offsetProcess;
    this->m_pixelFunctionId = SceneManager::Instance()->my_fs_texArrPass;
}

void MyIndirectRenderer::render()
{
    glBindVertexArray(this->vao);
    glActiveTexture(SceneManager::Instance()->my_albedoTexArrUnit);
    glBindTexture(GL_TEXTURE_2D_ARRAY, this->textureHandle);

    glUniformMatrix4fv(SceneManager::Instance()->m_modelMatHandle, 1, false, glm::value_ptr(this->model));
    glUniform1i(SceneManager::Instance()->m_vs_vertexProcessIdHandle, this->m_vertexFunctionId);
    glUniform1i(SceneManager::Instance()->m_fs_pixelProcessIdHandle, this->m_pixelFunctionId);

    glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (const void*)0, this->meshCount, 0);
}

void MyIndirectRenderer::resetRender()
{
    glBindVertexArray(this->vao);
    glDispatchCompute(this->meshCount, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void MyIndirectRenderer::cullRender(glm::mat4 viewProjMat)
{
    glBindVertexArray(this->vao);
    glUniformMatrix4fv(SceneManager::Instance()->my_viewProjMatLocation, 1, false, glm::value_ptr(viewProjMat));
    glUniform1ui(SceneManager::Instance()->my_maxInsLocation, offsetCnt);
    glDispatchCompute(offsetCnt / 1024 + 1, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}
