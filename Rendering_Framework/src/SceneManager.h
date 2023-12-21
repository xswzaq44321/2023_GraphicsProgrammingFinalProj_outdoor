#pragma once

#include <glad\glad.h>

// Singleton

class SceneManager
{
private:
	SceneManager(){}
	

public:	

	virtual ~SceneManager() {}

	static SceneManager *Instance() {
		static SceneManager *m_instance = nullptr;
		if (m_instance == nullptr) {
			m_instance = new SceneManager();
		}
		return m_instance;
	}
	
	GLuint m_vertexHandle;
	GLuint m_normalHandle;
	GLuint m_uvHandle;
    unsigned int my_offsetHandle;
    unsigned int my_rotationHandle;

	GLuint m_modelMatHandle;
	GLuint m_viewMatHandle;
	GLuint m_projMatHandle;
	GLuint m_terrainVToUVMatHandle;

    unsigned int my_maxInsLocation;

	GLuint m_albedoMapHandle;
	GLuint m_normalMapHandle;
	GLuint m_elevationMapHandle;
    unsigned int my_albedoTexArrHandle;
	
	GLuint m_fs_pixelProcessIdHandle;
	GLuint m_vs_vertexProcessIdHandle;

	GLenum m_albedoTexUnit;
	GLenum m_normalTexUnit;
	GLenum m_elevationTexUnit;
    GLenum my_albedoTexArrUnit;


	int m_albedoMapTexIdx;
	int m_elevationMapTexIdx;
	int m_normalMapTexIdx;
    int my_albedoMapTexArrIdx;

	int m_vs_commonProcess;
    int m_vs_terrainProcess;
    int my_vs_offsetProcess;
	
	int m_fs_pureColor;	
	int m_fs_terrainPass;
    int my_fs_texturePass;
    int my_fs_stonePass;
    int my_fs_texArrPass;
};

