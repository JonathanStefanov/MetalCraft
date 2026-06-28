#pragma once
#include "../texture/manager/TextureManager.h"

enum class ItemType {
    NONE,
    DIRT,
    WOOD,
    LEAF,
    GLOW_STONE,
    GRASS
};

struct Item {
    ItemType type;
    int quantity;

    Item(ItemType type = ItemType::NONE, int quantity = 0) : type(type), quantity(quantity) {}
    TextureType getTextureType() const;
    static Item fromTextureType(TextureType textureType, int quantity = 1);
};
