#include "DroppedItemManager.h"

#include "../../objects/player/Player.h"

void DroppedItemManager::spawn(const Item& item, glm::vec3 position, Shader& shader) {
    if (item.type == ItemType::NONE || item.quantity <= 0) {
        return;
    }

    auto droppedItem = std::make_unique<DroppedItem>(item, position);
    droppedItem->makeObject(shader);
    droppedItems.push_back(std::move(droppedItem));
}

void DroppedItemManager::update(Player& player, World& world) {
    for (auto itemIt = droppedItems.begin(); itemIt != droppedItems.end();) {
        DroppedItem& droppedItem = **itemIt;
        droppedItem.update(world, player);

        if (droppedItem.canPickup(player)) {
            if (player.inventory.add(droppedItem.getItem())) {
                itemIt = droppedItems.erase(itemIt);
            } else {
                ++itemIt;
            }
        } else {
            ++itemIt;
        }
    }
}

void DroppedItemManager::draw(Shader& shader) {
    for (auto& droppedItem : droppedItems) {
        droppedItem->draw(shader);
    }
}
