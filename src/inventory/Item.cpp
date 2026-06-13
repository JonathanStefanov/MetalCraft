#include "Item.h"

TextureType Item::getTextureType() const {
    switch (type) {
        case ItemType::DIRT: return TextureType::DIRT;
        case ItemType::WOOD: return TextureType::WOOD;
        case ItemType::LEAF: return TextureType::LEAF;
        case ItemType::GLOW_STONE: return TextureType::GLOW_STONE;
        case ItemType::GRASS: return TextureType::GRASS;
        default: return TextureType::DIRT;
    }
}
