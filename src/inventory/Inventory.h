#pragma once
#include "Item.h"
#include <vector>

class Inventory {
public:
    static const int HOTBAR_SIZE = 9;
    
    Inventory();
    
    void add(const Item& item);
    Item getHotbarItem(int index) const;
    void setHotbarItem(int index, const Item& item);
    
    int selectedSlot = 0;
    
    Item getSelectedItem() const { return getHotbarItem(selectedSlot); }

private:
    std::vector<Item> hotbar;
};
