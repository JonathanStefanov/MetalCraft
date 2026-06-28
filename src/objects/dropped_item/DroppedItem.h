#pragma once

#include "../../inventory/Item.h"
#include "../game_object/GameObject.h"

class Player;
class World;

class DroppedItem : public GameObject {
public:
    DroppedItem(const Item& item, glm::vec3 position);

    void update(World& world);
    bool canPickup(Player& player) const;
    const Item& getItem() const;

private:
    Item item;
    float verticalVelocity = 0.08f;
    float pickupRadius = 1.25f;
};
