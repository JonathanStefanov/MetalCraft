#include "Inventory.h"
#include <algorithm>

Inventory::Inventory() {
    hotbar.resize(HOTBAR_SIZE);
}

bool Inventory::add(const Item& item) {
    if (item.type == ItemType::NONE || item.quantity <= 0) return false;

    int remaining = item.quantity;
    
    for (int i = 0; i < HOTBAR_SIZE; i++) {
        if (hotbar[i].type == item.type && hotbar[i].quantity < MAX_STACK_SIZE) {
            int availableSpace = MAX_STACK_SIZE - hotbar[i].quantity;
            int toAdd = std::min(availableSpace, remaining);
            hotbar[i].quantity += toAdd;
            remaining -= toAdd;
            if (remaining == 0) {
                return true;
            }
        }
    }
    
    for (int i = 0; i < HOTBAR_SIZE; i++) {
        if (hotbar[i].type == ItemType::NONE) {
            int toAdd = std::min(MAX_STACK_SIZE, remaining);
            hotbar[i] = Item(item.type, toAdd);
            remaining -= toAdd;
            if (remaining == 0) {
                return true;
            }
        }
    }

    return false;
}

bool Inventory::remove(ItemType type, int quantity) {
    if (type == ItemType::NONE || quantity <= 0) return false;

    int available = 0;
    for (const Item& item : hotbar) {
        if (item.type == type) {
            available += item.quantity;
        }
    }
    if (available < quantity) {
        return false;
    }

    int remaining = quantity;
    for (Item& item : hotbar) {
        if (item.type == type) {
            int toRemove = std::min(item.quantity, remaining);
            item.quantity -= toRemove;
            remaining -= toRemove;
            if (item.quantity <= 0) {
                item = Item();
            }
            if (remaining == 0) {
                return true;
            }
        }
    }

    return true;
}

bool Inventory::consumeSelected(int quantity) {
    if (selectedSlot < 0 || selectedSlot >= HOTBAR_SIZE || quantity <= 0) {
        return false;
    }

    Item& selected = hotbar[selectedSlot];
    if (selected.type == ItemType::NONE || selected.quantity < quantity) {
        return false;
    }

    selected.quantity -= quantity;
    if (selected.quantity <= 0) {
        selected = Item();
    }

    return true;
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
