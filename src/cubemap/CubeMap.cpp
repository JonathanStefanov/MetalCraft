//
// Created by gaspa on 05/01/2023.
//

#include <string>
#include <map>
#include <iostream>
#include "CubeMap.h"
#include "stb_image.h"
#include "../utils/metal/MetalContext.h"
#include <Metal/Metal.hpp>

CubeMap::CubeMap(const std::string& pathSuffix, std::map<std::string, int> & faces) {
    stbi_set_flip_vertically_on_load(false);

    int width, height, nrChannels;
    unsigned char *data;
    
    MTL::TextureDescriptor* texDesc = MTL::TextureDescriptor::alloc()->init();
    texDesc->setTextureType(MTL::TextureTypeCube);
    texDesc->setPixelFormat(MTL::PixelFormatRGBA8Unorm);
    
    MTL::Device* device = MetalContext::get()->getDevice();
    bool descriptorSetup = false;
    
    // face names usually: right, left, top, bottom, front, back
    for (auto &face : faces) {
        std::string filename = pathSuffix + face.first;
        data = stbi_load(filename.c_str(), &width, &height, &nrChannels, 4);
        
        if (data) {
            if (!descriptorSetup) {
                texDesc->setWidth(width);
                texDesc->setHeight(height);
                texture = device->newTexture(texDesc);
                descriptorSetup = true;
            }
            
            MTL::Region region = MTL::Region::Make2D(0, 0, width, height);
            texture->replaceRegion(region, 0, face.second, data, width * 4, width * height * 4);
            stbi_image_free(data);
        } else {
            std::cout << "Cubemap texture failed to load at path: " << filename << std::endl;
        }
    }
    
    texDesc->release();
}

CubeMap::~CubeMap() {
    if (texture) texture->release();
    if (vertexBuffer) vertexBuffer->release();
}

void CubeMap::makeObject(Shader& shader) {
    MTL::Device* device = MetalContext::get()->getDevice();
    vertexBuffer = device->newBuffer(&FacesCoords[0], FacesCoords.size() * sizeof(float), MTL::ResourceStorageModeShared);
}

void CubeMap::draw(Shader& shader) const {
    MTL::RenderCommandEncoder* encoder = shader.encoder;
    if (!encoder) return;
    
    encoder->setVertexBytes(&shader.uniforms, sizeof(Uniforms), 1);
    encoder->setFragmentTexture(texture, 0);
    
    if (vertexBuffer) {
        encoder->setVertexBuffer(vertexBuffer, 0, 0);
        encoder->drawPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger(0), NS::UInteger(36));
    }
}

std::vector<float> CubeMap::FacesCoords = {
// positions
-1.0f,  1.0f, -1.0f,
-1.0f, -1.0f, -1.0f,
1.0f, -1.0f, -1.0f,
1.0f, -1.0f, -1.0f,
1.0f,  1.0f, -1.0f,
-1.0f,  1.0f, -1.0f,

-1.0f, -1.0f,  1.0f,
-1.0f, -1.0f, -1.0f,
-1.0f,  1.0f, -1.0f,
-1.0f,  1.0f, -1.0f,
-1.0f,  1.0f,  1.0f,
-1.0f, -1.0f,  1.0f,

1.0f, -1.0f, -1.0f,
1.0f, -1.0f,  1.0f,
1.0f,  1.0f,  1.0f,
1.0f,  1.0f,  1.0f,
1.0f,  1.0f, -1.0f,
1.0f, -1.0f, -1.0f,

-1.0f, -1.0f,  1.0f,
-1.0f,  1.0f,  1.0f,
1.0f,  1.0f,  1.0f,
1.0f,  1.0f,  1.0f,
1.0f, -1.0f,  1.0f,
-1.0f, -1.0f,  1.0f,

-1.0f,  1.0f, -1.0f,
1.0f,  1.0f, -1.0f,
1.0f,  1.0f,  1.0f,
1.0f,  1.0f,  1.0f,
-1.0f,  1.0f,  1.0f,
-1.0f,  1.0f, -1.0f,

-1.0f, -1.0f, -1.0f,
-1.0f, -1.0f,  1.0f,
1.0f, -1.0f, -1.0f,
1.0f, -1.0f, -1.0f,
-1.0f, -1.0f,  1.0f,
1.0f, -1.0f,  1.0f
};