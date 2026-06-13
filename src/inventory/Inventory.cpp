#include "Inventory.h"

Inventory::Inventory() {
    hotbar.resize(HOTBAR_SIZE);
}

void Inventory::add(const Item& item) {
    if (item.type == ItemType::NONE) return;
    
    for (int i = 0; i < HOTBAR_SIZE; i++) {
        if (hotbar[i].type == item.type) {
            hotbar[i].quantity += item.quantity;
            return;
        }
    }
    
    for (int i = 0; i < HOTBAR_SIZE; i++) {
        if (hotbar[i].type == ItemType::NONE) {
            hotbar[i] = item;
            return;
        }
    }
}

Item Inventory::getHotbarItem(int index) const {
    if (index >= 0 && index < HOTBAR_SIZE) {
        return hotbar[index];
    }
    return Item();
}

void Inventory::setHotbarItem(int index, const Item& item) {
    if (index >= 0 && index < HOTBAR_SIZE) {
        hotbar[index] = item;
    }
}
