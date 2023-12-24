#include "SceneRenderer.h"


SceneRenderer::SceneRenderer()
{
}


SceneRenderer::~SceneRenderer()
{
}
void SceneRenderer::startNewFrame() {
	this->m_shaderProgram->useProgram();
	this->clear();
}
void SceneRenderer::computeRenderPass()
{
    // reset for compute shader
    this->m_resetShaderProgram->useProgram();
    this->my_indirectSO->resetRender();

    // compute shading
    this->m_computeshaderProgram->useProgram();
    this->my_indirectSO->cullRender(this->m_projMat, this->m_viewMat, this->m_frameWidth / 2.0 / this->m_frameHeight);
}
void SceneRenderer::renderPass(){
	SceneManager *manager = SceneManager::Instance();	

    // render to screen
    this->m_shaderProgram->useProgram();
	glUniformMatrix4fv(manager->m_projMatHandle, 1, false, glm::value_ptr(this->m_projMat));
	glUniformMatrix4fv(manager->m_viewMatHandle, 1, false, glm::value_ptr(this->m_viewMat));

	if (this->m_terrainSO != nullptr) {
		glUniform1i(SceneManager::Instance()->m_vs_vertexProcessIdHandle, SceneManager::Instance()->m_vs_terrainProcess);
		this->m_terrainSO->update();
	}

	if (this->m_dynamicSOs.size() > 0) {
		glUniform1i(SceneManager::Instance()->m_vs_vertexProcessIdHandle, SceneManager::Instance()->m_vs_commonProcess);
		for (DynamicSceneObject *obj : this->m_dynamicSOs) {
			obj->update();
		}
	}
    if (this->my_indirectSO != nullptr) {
        this->my_indirectSO->render();
    }
	if (this->m_magicStone != nullptr) {
		this->m_magicStone->update();
	}
	
}

// =======================================
void SceneRenderer::resize(const int w, const int h){
	this->m_frameWidth = w;
	this->m_frameHeight = h;
}
bool SceneRenderer::initialize(const int w, const int h, ShaderProgram* shaderProgram, ShaderProgram* resetShaderProgram, ShaderProgram* computeShaderProgram){
	this->m_shaderProgram = shaderProgram;
    this->m_resetShaderProgram = resetShaderProgram;
    this->m_computeshaderProgram = computeShaderProgram;

	this->resize(w, h);
	const bool flag = this->setUpShader();
	
	if (!flag) {
		return false;
	}	
	
	glEnable(GL_DEPTH_TEST);

	return true;
}
void SceneRenderer::setProjection(const glm::mat4 &proj){
	this->m_projMat = proj;
}
void SceneRenderer::setView(const glm::mat4 &view){
	this->m_viewMat = view;
}
void SceneRenderer::setViewport(const int x, const int y, const int w, const int h) {
	glViewport(x, y, w, h);
}
void SceneRenderer::appendDynamicSceneObject(DynamicSceneObject *obj){
	this->m_dynamicSOs.push_back(obj);
}
void SceneRenderer::appendTerrainSceneObject(TerrainSceneObject* tSO) {
	this->m_terrainSO = tSO;
}
void SceneRenderer::appendIndirectSceneObject(MyIndirectRenderer * obj)
{
    this->my_indirectSO = obj;
}
void SceneRenderer::appendMagicStoneSceneObject(MagicRock * obj)
{
	this->m_magicStone = obj;
}
void SceneRenderer::clear(const glm::vec4 &clearColor, const float depth){
	static const float COLOR[] = { 0.0, 0.0, 0.0, 1.0 };
	static const float DEPTH[] = { 1.0 };

	glClearBufferfv(GL_COLOR, 0, COLOR);
	glClearBufferfv(GL_DEPTH, 0, DEPTH);
}
bool SceneRenderer::setUpShader(){
	if (this->m_shaderProgram == nullptr) {
		return false;
	}

	this->m_shaderProgram->useProgram();

	// shader attributes binding
	const GLuint programId = this->m_shaderProgram->programId();

	SceneManager *manager = SceneManager::Instance();
	manager->m_vertexHandle = 0;
	manager->m_normalHandle = 1;
	manager->m_uvHandle = 2;
    manager->my_offsetHandle = 3;
    manager->my_tangentHandle = 4;
	manager->my_bitangentHandle = 5;

	// =================================
	manager->m_modelMatHandle = 0;
	manager->m_viewMatHandle = 7;
	manager->m_projMatHandle = 8;
	manager->m_terrainVToUVMatHandle = 9;

    // compute shader attributes
    manager->my_viewProjMatLocation = 0;
    manager->my_maxInsLocation = 1;
    manager->my_viewMatLocation = 2;
    manager->my_nearFaceLocation = 3;
    manager->my_farFaceLocation = 4;
    manager->my_rightFaceLocation = 5;
    manager->my_leftFaceLocation = 6;
    manager->my_topFaceLocation = 7;
    manager->my_bottomFaceLocation = 8;

    manager->my_instanceBind = 1;
    manager->my_validBind = 2;
    manager->my_drawCmdBind = 3;
    // =================================

	manager->m_albedoMapHandle = 4;
	manager->m_albedoMapTexIdx = 0;
	glUniform1i(manager->m_albedoMapHandle, manager->m_albedoMapTexIdx);

	manager->m_elevationMapHandle = 5;
	manager->m_elevationMapTexIdx = 3;
	glUniform1i(manager->m_elevationMapHandle, manager->m_elevationMapTexIdx);
	
	manager->m_normalMapHandle = 6;
	manager->m_normalMapTexIdx = 2;
	glUniform1i(manager->m_normalMapHandle, manager->m_normalMapTexIdx);

    manager->m_albedoMapHandle = 10;
    manager->m_albedoMapTexIdx = 4;
    glUniform1i(manager->m_albedoMapHandle, manager->m_albedoMapTexIdx);
	
	manager->m_albedoTexUnit = GL_TEXTURE0;
	manager->m_elevationTexUnit = GL_TEXTURE3;
	manager->m_normalTexUnit = GL_TEXTURE2;
    manager->my_albedoTexArrUnit = GL_TEXTURE4;

	manager->m_vs_vertexProcessIdHandle = 1;
	manager->m_vs_commonProcess = 0;
	manager->m_vs_terrainProcess = 3;
    manager->my_vs_offsetProcess = 4;

	manager->m_fs_pixelProcessIdHandle = 2;
	manager->m_fs_pureColor = 5;
	manager->m_fs_terrainPass = 7;
    manager->my_fs_texturePass = 9;
    manager->my_fs_stonePass = 10;
	manager->my_fs_stonePassTex = 13;
    manager->my_fs_texArrPass = 11;
	manager->my_fs_planePass = 12;
	
	return true;
}
