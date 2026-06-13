//
// Created by gaspa on 27/12/2022.
//

#include "Renderer.h"
#include "../../texture/manager/TextureManager.h"
#include "../../utils/metal/MetalContext.h"
#include <Metal/Metal.hpp>

void Renderer::draw(Shader &shader, Transform &transform, Mesh *mesh, MTL::Texture* texture) const {
    MTL::RenderCommandEncoder* encoder = shader.encoder;
    if (!encoder) return;
    
    shader.setMatrix4("M", transform.getModel());

    // Send uniforms
    encoder->setVertexBytes(&shader.uniforms, sizeof(Uniforms), 1);
    encoder->setFragmentBytes(&shader.uniforms, sizeof(Uniforms), 1);
    
    if (shader.withLight) {
        encoder->setFragmentBytes(&shader.lightUniforms, sizeof(LightUniforms), 2);
    }
    
    if (shader.withTexture && texture) {
        encoder->setFragmentTexture(texture, 0);
    }

    if (vertexBuffer) {
        encoder->setVertexBuffer(vertexBuffer, 0, 0);
        encoder->drawPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger(0), NS::UInteger(mesh->getVerticesCount()));
    }
}

void Renderer::makeObject(Shader &shader, Mesh *mesh, Transform &transform) {
    if (!vertexBuffer) {
        float *data = mesh->toFloatArray();
        int dataSize = mesh->getFloatArraySize();

        MTL::Device* device = MetalContext::get()->getDevice();
        vertexBuffer = device->newBuffer(data, dataSize, MTL::ResourceStorageModeShared);
        
        delete[] data;
    }
}

void Renderer::makeObject(Shader& shader, Mesh *pMesh, Transform& transform, Renderer& renderer) {
    vertexBuffer = renderer.vertexBuffer;
    makeObject(shader, pMesh, transform);
}
