#pragma once

#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include "MyPoissonSample.h"
#include "SceneManager.h"

struct DrawElementsIndirectCommand {
    unsigned int count;
    unsigned int instanceCount;
    unsigned int firstIndex;
    unsigned int baseVertex;
    unsigned int baseInstance;
};

struct InstanceProperties {
    glm::vec4 position;
    glm::mat4 rotation;
};

class MyIndirectRenderer
{
public:
    unsigned int textureHandle;
    std::vector<int> index_count;
    std::vector<const void*> indices_begin;
    std::vector<int> baseVertices;
    std::vector<DrawElementsIndirectCommand> cmds;

    MyIndirectRenderer();
    MyIndirectRenderer(const MyIndirectRenderer&) = delete;
    MyIndirectRenderer(MyIndirectRenderer&&) = delete;

    void render();
    void resetRender();
    void cullRender(glm::mat4 viewProjMat);

private:
    unsigned int vao;
    int m_pixelFunctionId;
    int m_vertexFunctionId;

    glm::mat4 model = glm::mat4(1.0f);

    int offsetCnt;
    int meshCount;
};