#pragma once

#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "SceneManager.h"

#include <optional>

class DynamicSceneObject 
{
public:
    static GLuint createTexture(const void *data, const int numComp, const int width, const int height, const GLint internalFormat, const GLenum imageFormat, const GLenum type, const GLint wrapMode, const GLint minMagFilter);

private:
	GLuint m_indexBufferHandle;
	float *m_dataBuffer = nullptr;
	unsigned int *m_indexBuffer = nullptr;

	GLuint m_vao;
	GLuint m_dataBufferHandle;
	GLenum m_primitive;
	int m_pixelFunctionId;
	int m_vertexFunctionId;
	int m_indexCount;
	int strideV;
    std::optional<GLuint> my_albedoMapHandle;
    std::optional<GLuint> my_normalMapHandle;

	glm::mat4 m_modelMat;

public:
	DynamicSceneObject(const int maxNumVertex, const int maxNumIndex, const bool normalFlag, const bool uvFlag, const bool tangentAndBitangent = false);
	virtual ~DynamicSceneObject();

	void update();

	float* dataBuffer();
	unsigned int *indexBuffer();

	void updateDataBuffer(const int byteOffset, const int dataByte);
	void updateIndexBuffer(const int byteOffset, const int dataByte);

	void setPixelFunctionId(const int functionId);
	void setVertexFunctionId(const int functionId);
	void setPrimitive(const GLenum primitive);
	void setModelMat(const glm::mat4& modelMat);
    void setAlbedoTex(GLuint albedoHandle);
    void setNormalTex(GLuint normalHandle);

	int getStrideV() { return strideV; };
};

