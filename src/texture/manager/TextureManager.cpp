//
// Created by gaspa on 28/12/2022.
//

#include <iostream>
#include "TextureManager.h"
#include "../texture/Texture.h"
#include <Metal/Metal.hpp>

std::map<TextureType, MTL::Texture*> TextureManager::textures;

void TextureManager::linkTexture(TextureType type, const char *path) {
    Texture t(path);
    if (t.getTexture()) {
        textures[type] = t.getTexture();
        textures[type]->retain();
    }
}

MTL::Texture* TextureManager::getTextureID(TextureType type) {
    if (textures.find(type) != textures.end()) {
        return textures[type];
    } else {
        return nullptr;
    }
}