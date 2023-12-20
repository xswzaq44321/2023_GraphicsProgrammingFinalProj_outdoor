#include "DynamicSceneObject.h"


GLuint DynamicSceneObject::createTexture(const void * data, const int numComp, const int width, const int height, const GLint internalFormat, const GLenum imageFormat, const GLenum type, const GLint wrapMode, const GLint minMagFilter)
{
    // use fast 4-byte alignment (default anyway) if possible
    if ((width * numComp) % 4 != 0) {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    }

    // Create Texture
    GLuint texHandle;
    glGenTextures(1, &texHandle);
    glBindTexture(GL_TEXTURE_2D, texHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, imageFormat, type, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minMagFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, minMagFilter);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);

    glBindTexture(GL_TEXTURE_2D, 0);

    return texHandle;
}

DynamicSceneObject::DynamicSceneObject(const int maxNumVertex, const int maxNumIndex, const bool normalFlag, const bool uvFlag)
{
    // the data should be INTERLEAF format

    int totalBufferDataByte = maxNumVertex * 12;
    int strideV = 3;
    if (normalFlag == true) {
        totalBufferDataByte += maxNumVertex * 12;
        strideV += 3;
    }
    if (uvFlag == true) {
        totalBufferDataByte += maxNumVertex * 12;
        strideV += 3;
    }

    this->m_indexCount = maxNumIndex;

    this->m_dataBuffer = new float[totalBufferDataByte / 4];
    this->m_indexBuffer = new unsigned int[maxNumIndex];

    // Create Geometry Data Buffer
    glCreateBuffers(1, &(this->m_dataBufferHandle));
    glNamedBufferData(this->m_dataBufferHandle, totalBufferDataByte, this->m_dataBuffer, GL_DYNAMIC_DRAW);

    // Create Indices Buffer
    glCreateBuffers(1, &m_indexBufferHandle);
    glNamedBufferData(m_indexBufferHandle, maxNumIndex * 4, this->m_indexBuffer, GL_DYNAMIC_DRAW);

    // create vao
    glGenVertexArrays(1, &(this->m_vao));
    glBindVertexArray(this->m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, this->m_dataBufferHandle);
    int byteOffset = 0;
    glVertexAttribPointer(SceneManager::Instance()->m_vertexHandle, 3, GL_FLOAT, false, strideV * 4, (void*)(byteOffset));
    byteOffset = byteOffset + 12;
    glEnableVertexAttribArray(SceneManager::Instance()->m_vertexHandle);
    if (normalFlag) {
        glVertexAttribPointer(SceneManager::Instance()->m_normalHandle, 3, GL_FLOAT, false, strideV * 4, (void*)(byteOffset));
        byteOffset = byteOffset + 12;
        glEnableVertexAttribArray(SceneManager::Instance()->m_normalHandle);
    }
    if (uvFlag) {
        glVertexAttribPointer(SceneManager::Instance()->m_uvHandle, 3, GL_FLOAT, false, strideV * 4, (void*)(byteOffset));
        byteOffset = byteOffset + 12;
        glEnableVertexAttribArray(SceneManager::Instance()->m_uvHandle);
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferHandle);
    glBindVertexArray(0);
}


DynamicSceneObject::~DynamicSceneObject()
{
    delete[] this->m_dataBuffer;
    delete[] this->m_indexBuffer;
}

void DynamicSceneObject::update() {
    // bind Buffer
    glBindVertexArray(this->m_vao);

    if (this->my_albedoMapHandle.has_value()) {
        glActiveTexture(SceneManager::Instance()->m_albedoTexUnit);
        glBindTexture(GL_TEXTURE_2D, this->my_albedoMapHandle.value());
    }

    if (this->my_normalMapHandle.has_value()) {
        glActiveTexture(SceneManager::Instance()->m_normalTexUnit);
        glBindTexture(GL_TEXTURE_2D, this->my_normalMapHandle.value());
    }

    // model matrix
    glUniformMatrix4fv(SceneManager::Instance()->m_modelMatHandle, 1, false, glm::value_ptr(this->m_modelMat));

    glUniform1i(SceneManager::Instance()->m_fs_pixelProcessIdHandle, this->m_pixelFunctionId);
    glDrawElements(this->m_primitive, this->m_indexCount, GL_UNSIGNED_INT, nullptr);
}

float* DynamicSceneObject::dataBuffer() { return this->m_dataBuffer; }
unsigned int *DynamicSceneObject::indexBuffer() { return this->m_indexBuffer; }

void DynamicSceneObject::updateDataBuffer(const int byteOffset, const int dataByte) {
    float *data = this->m_dataBuffer + byteOffset / 4;
    glNamedBufferSubData(this->m_dataBufferHandle, byteOffset, dataByte, data);
}
void DynamicSceneObject::updateIndexBuffer(const int byteOffset, const int dataByte) {
    unsigned int *data = this->m_indexBuffer + byteOffset / 4;
    glNamedBufferSubData(this->m_indexBufferHandle, byteOffset, dataByte, data);
}

void DynamicSceneObject::setPixelFunctionId(const int functionId) {
    this->m_pixelFunctionId = functionId;
}
void DynamicSceneObject::setPrimitive(const GLenum primitive) {
    this->m_primitive = primitive;
}
void DynamicSceneObject::setModelMat(const glm::mat4& modelMat) {
    this->m_modelMat = modelMat;
}

void DynamicSceneObject::setAlbedoTex(GLuint albedoHandle)
{
    this->my_albedoMapHandle = albedoHandle;
}

void DynamicSceneObject::setNormalTex(GLuint normalHandle)
{
    this->my_normalMapHandle = normalHandle;
}
