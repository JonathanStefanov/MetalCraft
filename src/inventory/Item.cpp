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

Item Item::fromTextureType(TextureType textureType, int quantity) {
    switch (textureType) {
        case TextureType::DIRT: return Item(ItemType::DIRT, quantity);
        case TextureType::WOOD: return Item(ItemType::WOOD, quantity);
        case TextureType::LEAF: return Item(ItemType::LEAF, quantity);
        case TextureType::GLOW_STONE: return Item(ItemType::GLOW_STONE, quantity);
        case TextureType::GRASS: return Item(ItemType::GRASS, quantity);
        default: return Item();
    }
}
