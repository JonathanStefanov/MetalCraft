#include "UIUtils.h"
#include "Font.h" // For UIVertex
#include "../utils/metal/MetalContext.h"
#include "../utils/shader/shader/Shader.h"
#include <Metal/Metal.hpp>
#include <glm/gtc/type_ptr.hpp>

struct UIUniforms {
    simd::float4x4 projection;
    simd::float4 color;
    int type;
    int padding[3];
};

void UIUtils::drawRect(Shader& shader, float x, float y, float w, float h, const glm::vec4& color) {
    MTL::RenderCommandEncoder* encoder = shader.encoder;
    if (!encoder) return;

    UIVertex vertices[] = {
        {{x, y}, {0, 0}},
        {{x + w, y}, {1, 0}},
        {{x, y + h}, {0, 1}},
        {{x, y + h}, {0, 1}},
        {{x + w, y}, {1, 0}},
        {{x + w, y + h}, {1, 1}}
    };

    MTL::Device* device = MetalContext::get()->getDevice();
    MTL::Buffer* vertexBuffer = device->newBuffer(vertices, sizeof(vertices), MTL::ResourceStorageModeShared);

    UIUniforms uniforms;
    memcpy(&uniforms.projection, glm::value_ptr(shader.uiProjection), sizeof(simd::float4x4));
    uniforms.color = simd_make_float4(color.r, color.g, color.b, color.a);
    uniforms.type = 0; // Solid

    encoder->setVertexBytes(&uniforms, sizeof(UIUniforms), 1);
    encoder->setFragmentBytes(&uniforms, sizeof(UIUniforms), 1);

    encoder->setVertexBuffer(vertexBuffer, 0, 0);
    encoder->drawPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger(0), NS::UInteger(6));

    vertexBuffer->release();
}

void UIUtils::drawTexture(Shader& shader, MTL::Texture* texture, float x, float y, float w, float h, const glm::vec4& color) {
    MTL::RenderCommandEncoder* encoder = shader.encoder;
    if (!encoder || !texture) return;

    UIVertex vertices[] = {
        {{x, y}, {0, 0}},
        {{x + w, y}, {1, 0}},
        {{x, y + h}, {0, 1}},
        {{x, y + h}, {0, 1}},
        {{x + w, y}, {1, 0}},
        {{x + w, y + h}, {1, 1}}
    };

    MTL::Device* device = MetalContext::get()->getDevice();
    MTL::Buffer* vertexBuffer = device->newBuffer(vertices, sizeof(vertices), MTL::ResourceStorageModeShared);

    UIUniforms uniforms;
    memcpy(&uniforms.projection, glm::value_ptr(shader.uiProjection), sizeof(simd::float4x4));
    uniforms.color = simd_make_float4(color.r, color.g, color.b, color.a);
    uniforms.type = 2; // Image

    encoder->setFragmentTexture(texture, 0);

    encoder->setVertexBytes(&uniforms, sizeof(UIUniforms), 1);
    encoder->setFragmentBytes(&uniforms, sizeof(UIUniforms), 1);

    encoder->setVertexBuffer(vertexBuffer, 0, 0);
    encoder->drawPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger(0), NS::UInteger(6));

    vertexBuffer->release();
}
