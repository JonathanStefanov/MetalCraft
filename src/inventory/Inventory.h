#pragma once
#include "Item.h"
#include <vector>

class Inventory {
public:
    static const int HOTBAR_SIZE = 9;
    static constexpr int MAX_STACK_SIZE = 64;
    
    Inventory();
    
    bool add(const Item& item);
    bool remove(ItemType type, int quantity);
    bool consumeSelected(int quantity = 1);
    Item getHotbarItem(int index) const;
    void setHotbarItem(int index, const Item& item);
    
    int selectedSlot = 0;
    
    Item getSelectedItem() const { return getHotbarItem(selectedSlot); }

private:
    std::vector<Item> hotbar;
};
