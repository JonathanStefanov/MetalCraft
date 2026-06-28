#include "BlockOutlineRenderer.h"

#include "../utils/metal/MetalContext.h"
#include <Metal/Metal.hpp>
#include <glm/gtc/matrix_transform.hpp>

BlockOutlineRenderer::BlockOutlineRenderer() {
    const float minX = -0.505f;
    const float maxX = 0.505f;
    const float minY = -0.005f;
    const float maxY = 1.015f;
    const float minZ = -0.505f;
    const float maxZ = 0.505f;

    const float vertices[][8] = {
            {minX, minY, minZ, 0, 0, 0, 0, 0}, {maxX, minY, minZ, 0, 0, 0, 0, 0},
            {maxX, minY, minZ, 0, 0, 0, 0, 0}, {maxX, minY, maxZ, 0, 0, 0, 0, 0},
            {maxX, minY, maxZ, 0, 0, 0, 0, 0}, {minX, minY, maxZ, 0, 0, 0, 0, 0},
            {minX, minY, maxZ, 0, 0, 0, 0, 0}, {minX, minY, minZ, 0, 0, 0, 0, 0},

            {minX, maxY, minZ, 0, 0, 0, 0, 0}, {maxX, maxY, minZ, 0, 0, 0, 0, 0},
            {maxX, maxY, minZ, 0, 0, 0, 0, 0}, {maxX, maxY, maxZ, 0, 0, 0, 0, 0},
            {maxX, maxY, maxZ, 0, 0, 0, 0, 0}, {minX, maxY, maxZ, 0, 0, 0, 0, 0},
            {minX, maxY, maxZ, 0, 0, 0, 0, 0}, {minX, maxY, minZ, 0, 0, 0, 0, 0},

            {minX, minY, minZ, 0, 0, 0, 0, 0}, {minX, maxY, minZ, 0, 0, 0, 0, 0},
            {maxX, minY, minZ, 0, 0, 0, 0, 0}, {maxX, maxY, minZ, 0, 0, 0, 0, 0},
            {maxX, minY, maxZ, 0, 0, 0, 0, 0}, {maxX, maxY, maxZ, 0, 0, 0, 0, 0},
            {minX, minY, maxZ, 0, 0, 0, 0, 0}, {minX, maxY, maxZ, 0, 0, 0, 0, 0},
    };

    vertexBuffer = MetalContext::get()->getDevice()->newBuffer(vertices, sizeof(vertices), MTL::ResourceStorageModeShared);
}

BlockOutlineRenderer::~BlockOutlineRenderer() {
    if (vertexBuffer) {
        vertexBuffer->release();
    }
}

void BlockOutlineRenderer::draw(Shader& shader, const glm::vec3& blockPosition) {
    MTL::RenderCommandEncoder* encoder = shader.encoder;
    if (!encoder || !vertexBuffer) return;

    glm::mat4 model = glm::translate(glm::mat4(1.0f), blockPosition);
    shader.setMatrix4("M", model);

    encoder->setVertexBytes(&shader.uniforms, sizeof(Uniforms), 1);
    encoder->setFragmentBytes(&shader.uniforms, sizeof(Uniforms), 1);
    encoder->setVertexBuffer(vertexBuffer, 0, 0);
    encoder->drawPrimitives(MTL::PrimitiveTypeLine, NS::UInteger(0), NS::UInteger(24));
}
