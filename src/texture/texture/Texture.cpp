//
// Created by gaspa on 28/12/2022.
//

#include "Texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>
#include "../../utils/metal/MetalContext.h"
#include <Metal/Metal.hpp>

Texture::Texture(std::string path) {
    stbi_set_flip_vertically_on_load(false); // No need to flip vertically for Metal usually, but we will keep OpenGL conventions if needed. Actually Metal origin is top-left, OpenGL is bottom-left. Let's flip it if the original did to keep UVs working.
    // Wait, original code: stbi_set_flip_vertically_on_load(true);
    stbi_set_flip_vertically_on_load(true);

    int width, height, imNrChannels;
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &imNrChannels, 4);
    
    if (data) {
        MTL::TextureDescriptor* texDesc = MTL::TextureDescriptor::alloc()->init();
        texDesc->setPixelFormat(MTL::PixelFormatRGBA8Unorm);
        texDesc->setWidth(width);
        texDesc->setHeight(height);
        
        MTL::Device* device = MetalContext::get()->getDevice();
        texture = device->newTexture(texDesc);
        
        MTL::Region region = MTL::Region::Make2D(0, 0, width, height);
        texture->replaceRegion(region, 0, data, width * 4);
        
        texDesc->release();
        stbi_image_free(data);
    } else {
        std::cout << "Failed to load texture block: " << path << std::endl;
    }
}

Texture::~Texture() {
    if (texture) texture->release();
}

MTL::Texture* Texture::getTexture() const {
    return texture;
}
