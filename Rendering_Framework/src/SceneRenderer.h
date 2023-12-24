#pragma once

#include <vector>
#include "Shader.h"
#include "SceneManager.h"
#include "DynamicSceneObject.h"
#include "terrain\TerrainSceneObject.h"
#include "MyIndirectRenderer.h"
#include "MagicRock.h"


class SceneRenderer
{
public:
	SceneRenderer();
	virtual ~SceneRenderer();

private:
    ShaderProgram *m_shaderProgram = nullptr;
    ShaderProgram *m_resetShaderProgram = nullptr;
    ShaderProgram *m_computeshaderProgram = nullptr;
	glm::mat4 m_projMat;
	glm::mat4 m_viewMat;
	int m_frameWidth;
	int m_frameHeight;
    int my_defferedMode;

	std::vector<DynamicSceneObject*> m_dynamicSOs;
	TerrainSceneObject* m_terrainSO = nullptr;
    MyIndirectRenderer* my_indirectSO = nullptr;


public:
	void resize(const int w, const int h);
	bool initialize(const int w, const int h, ShaderProgram* shaderProgram, ShaderProgram* resetShaderProgram, ShaderProgram* computeShaderProgram);

	void setProjection(const glm::mat4 &proj);
	void setView(const glm::mat4 &view);
	void setViewport(const int x, const int y, const int w, const int h);
    void setDeferredMode(int mode) { my_defferedMode = mode; }
	void appendDynamicSceneObject(DynamicSceneObject *obj);
	void appendTerrainSceneObject(TerrainSceneObject* tSO);
    void appendIndirectSceneObject(MyIndirectRenderer* obj);

// pipeline
public:
	void startNewFrame();
    void computeRenderPass();
	void renderPass();

private:
	void clear(const glm::vec4 &clearColor = glm::vec4(0.0, 0.0, 0.0, 1.0), const float depth = 1.0);
	bool setUpShader();
};

