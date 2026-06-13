#include "Shader.h"
#include <iostream>
#include <cstring>
#include <fstream>
#include <sstream>
#include "../../metal/MetalContext.h"
#include "glm/gtc/type_ptr.hpp"

Shader::Shader(const std::string& vertexFunctionName, const std::string& fragmentFunctionName, bool withTexture, bool withLight) : withTexture(withTexture), withLight(withLight) {
    memset(&uniforms, 0, sizeof(Uniforms));
    uniforms.opacity = 1.0f;
    memset(&lightUniforms, 0, sizeof(LightUniforms));
    
    MTL::Device* device = MetalContext::get()->getDevice();
    
    NS::Error* error = nullptr;
    
    std::ifstream file("shaders/main.metal");
    if (!file.is_open()) {
        std::cout << "Failed to open shaders/main.metal\n";
        return;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();
    
    auto sourceNS = NS::String::string(source.c_str(), NS::UTF8StringEncoding);
    MTL::CompileOptions* options = MTL::CompileOptions::alloc()->init();
    
    MTL::Library* library = device->newLibrary(sourceNS, options, &error);
    options->release();
    
    if (!library) {
        std::cout << "Failed to compile the default library from source: ";
        if (error) std::cout << error->localizedDescription()->utf8String();
        std::cout << "\n";
        return;
    }
    
    auto vertexName = NS::String::string(vertexFunctionName.c_str(), NS::UTF8StringEncoding);
    auto fragmentName = NS::String::string(fragmentFunctionName.c_str(), NS::UTF8StringEncoding);
    
    MTL::Function* vertexFunc = library->newFunction(vertexName);
    MTL::Function* fragmentFunc = library->newFunction(fragmentName);
    
    MTL::RenderPipelineDescriptor* pipelineDesc = MTL::RenderPipelineDescriptor::alloc()->init();
    pipelineDesc->setVertexFunction(vertexFunc);
    pipelineDesc->setFragmentFunction(fragmentFunc);
    
    if (vertexFunctionName == "uiVertex") {
        pipelineDesc->colorAttachments()->object(0)->setPixelFormat(MTL::PixelFormatBGRA8Unorm);
        pipelineDesc->colorAttachments()->object(0)->setBlendingEnabled(true);
        pipelineDesc->colorAttachments()->object(0)->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
        pipelineDesc->colorAttachments()->object(0)->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
        pipelineDesc->colorAttachments()->object(0)->setSourceAlphaBlendFactor(MTL::BlendFactorSourceAlpha);
        pipelineDesc->colorAttachments()->object(0)->setDestinationAlphaBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
    } else {
        pipelineDesc->colorAttachments()->object(0)->setPixelFormat(MTL::PixelFormatBGRA8Unorm);
        pipelineDesc->setDepthAttachmentPixelFormat(MTL::PixelFormatDepth32Float);
    }
    
    MTL::VertexDescriptor* vertexDesc = MTL::VertexDescriptor::alloc()->init();
    if (vertexFunctionName == "cubemapVertex") {
        vertexDesc->attributes()->object(0)->setFormat(MTL::VertexFormatFloat3);
        vertexDesc->attributes()->object(0)->setOffset(0);
        vertexDesc->attributes()->object(0)->setBufferIndex(0);
        
        vertexDesc->layouts()->object(0)->setStride(3 * sizeof(float));
        vertexDesc->layouts()->object(0)->setStepFunction(MTL::VertexStepFunctionPerVertex);
    } else if (vertexFunctionName == "uiVertex") {
        vertexDesc->attributes()->object(0)->setFormat(MTL::VertexFormatFloat2);
        vertexDesc->attributes()->object(0)->setOffset(0);
        vertexDesc->attributes()->object(0)->setBufferIndex(0);
        
        vertexDesc->attributes()->object(1)->setFormat(MTL::VertexFormatFloat2);
        vertexDesc->attributes()->object(1)->setOffset(2 * sizeof(float));
        vertexDesc->attributes()->object(1)->setBufferIndex(0);
        
        vertexDesc->layouts()->object(0)->setStride(4 * sizeof(float));
        vertexDesc->layouts()->object(0)->setStepFunction(MTL::VertexStepFunctionPerVertex);
    } else {
        vertexDesc->attributes()->object(0)->setFormat(MTL::VertexFormatFloat3);
        vertexDesc->attributes()->object(0)->setOffset(0);
        vertexDesc->attributes()->object(0)->setBufferIndex(0);
        
        vertexDesc->attributes()->object(1)->setFormat(MTL::VertexFormatFloat2);
        vertexDesc->attributes()->object(1)->setOffset(3 * sizeof(float));
        vertexDesc->attributes()->object(1)->setBufferIndex(0);
        
        vertexDesc->attributes()->object(2)->setFormat(MTL::VertexFormatFloat3);
        vertexDesc->attributes()->object(2)->setOffset(5 * sizeof(float));
        vertexDesc->attributes()->object(2)->setBufferIndex(0);
        
        vertexDesc->layouts()->object(0)->setStride(8 * sizeof(float));
        vertexDesc->layouts()->object(0)->setStepFunction(MTL::VertexStepFunctionPerVertex);
    }
    
    pipelineDesc->setVertexDescriptor(vertexDesc);
    vertexDesc->release();
    
    
    // Setup blending for color attachment 0
    MTL::RenderPipelineColorAttachmentDescriptor* colorAttach = pipelineDesc->colorAttachments()->object(0);
    colorAttach->setBlendingEnabled(true);
    colorAttach->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
    colorAttach->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
    
    pipelineState = device->newRenderPipelineState(pipelineDesc, &error);
    if (!pipelineState) {
        std::cout << "Failed to create pipeline state: " << error->localizedDescription()->utf8String() << std::endl;
    }
    
    // Depth stencil state
    MTL::DepthStencilDescriptor* depthDesc = MTL::DepthStencilDescriptor::alloc()->init();
    if (vertexFunctionName == "uiVertex") {
        depthDesc->setDepthCompareFunction(MTL::CompareFunctionAlways);
        depthDesc->setDepthWriteEnabled(false);
    } else {
        depthDesc->setDepthCompareFunction(MTL::CompareFunctionLessEqual);
        depthDesc->setDepthWriteEnabled(true);
    }
    
    depthStencilState = device->newDepthStencilState(depthDesc);
    
    depthDesc->release();
    pipelineDesc->release();
    vertexFunc->release();
    fragmentFunc->release();
    library->release();
}

Shader::~Shader() {
    if (pipelineState) pipelineState->release();
    if (depthStencilState) depthStencilState->release();
}

void Shader::use() const {
    // Pipeline is set during rendering
}

void Shader::setInteger(const char *name, int value) {
    if (strcmp(name, "isWater") == 0) uniforms.isWater = value;
}

void Shader::setFloat(const char *name, float value) {
    if (strcmp(name, "opacity") == 0) uniforms.opacity = value;
    else if (strcmp(name, "shininess") == 0) lightUniforms.shininess = value;
    else if (strcmp(name, "light.ambient_strength") == 0) lightUniforms.ambient_strength = value;
    else if (strcmp(name, "light.diffuse_strength") == 0) lightUniforms.diffuse_strength = value;
    else if (strcmp(name, "light.specular_strength") == 0) lightUniforms.specular_strength = value;
    else if (strcmp(name, "light.constant") == 0) lightUniforms.constant_att = value;
    else if (strcmp(name, "light.linear") == 0) lightUniforms.linear = value;
    else if (strcmp(name, "light.quadratic") == 0) lightUniforms.quadratic = value;
}

void Shader::setVector3f(const char *name, float x, float y, float z) {
    if (strcmp(name, "u_view_pos") == 0) uniforms.u_view_pos = simd_make_float3(x, y, z);
    else if (strcmp(name, "light.light_pos") == 0) lightUniforms.light_pos = simd_make_float3(x, y, z);
}

void Shader::setVector3f(const char *name, const glm::vec3 &value) {
    setVector3f(name, value.x, value.y, value.z);
}

void Shader::setMatrix4(const char *name, const glm::mat4 &matrix) {
    simd::float4x4 m;
    memcpy(&m, glm::value_ptr(matrix), sizeof(simd::float4x4));
    
    if (strcmp(name, "M") == 0) uniforms.M = m;
    else if (strcmp(name, "V") == 0) uniforms.V = m;
    else if (strcmp(name, "P") == 0) uniforms.P = m;
    else if (strcmp(name, "lightSpaceMatrix") == 0) uniforms.lightSpaceMatrix = m;
}

void Shader::set1i(const char *name, int value) {
    setInteger(name, value);
}

void Shader::setBool(const char *name, bool b) {
    setInteger(name, b ? 1 : 0);
}

