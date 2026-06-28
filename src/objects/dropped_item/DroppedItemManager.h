#pragma once

#include "DroppedItem.h"
#include <memory>
#include <vector>

class Player;
class Shader;
class World;

class DroppedItemManager {
public:
    void spawn(const Item& item, glm::vec3 position, Shader& shader);
    void update(Player& player, World& world);
    void draw(Shader& shader);

private:
    std::vector<std::unique_ptr<DroppedItem>> droppedItems;
};
