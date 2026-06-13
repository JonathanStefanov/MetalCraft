#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <glm/glm.hpp>
#include "../../metal/ShaderTypes.h"

namespace MTL {
    class RenderPipelineState;
    class DepthStencilState;
    class RenderCommandEncoder;
}

class Shader
{
public:
    MTL::RenderPipelineState* pipelineState;
    MTL::DepthStencilState* depthStencilState;
    MTL::RenderCommandEncoder* encoder = nullptr;
    Uniforms uniforms;
    LightUniforms lightUniforms;
    glm::mat4 uiProjection;

    Shader(const std::string& vertexFunctionName, const std::string& fragmentFunctionName, bool withTexture = true, bool withLight = true);
    ~Shader();

    void use() const; // Unused in Metal, pipeline state is set during render encoding
    
    // Instead of set* methods sending to GPU, we update the local structs
    void setInteger(const char *name, int value);
    void setFloat(const char* name, float value);
    void setVector3f(const char* name, float x, float y, float z);
    void setVector3f(const char* name, const glm::vec3& value);
    void setMatrix4(const char* name, const glm::mat4& matrix);
    void set1i(const char* name, int value);
    void setBool(const char *string, bool b);

    bool withTexture, withLight;
};
#endif