#include "DroppedItem.h"

#include "../../objects/mesh/manager/MeshManager.h"
#include "../../objects/player/Player.h"
#include "../../texture/manager/TextureManager.h"
#include "../../world/World.h"
#include <glm/geometric.hpp>

DroppedItem::DroppedItem(const Item& item, glm::vec3 position)
        : GameObject(MeshManager::getMesh(MeshType::BLOCK)), item(item) {
    transform.setPosition(position.x, position.y, position.z);
    transform.setScale(0.35, 0.35, 0.35);
    transform.markAsDirtyState();
    collider = Collider{0.35f, 0.35f, 0.35f};
    setTexture(TextureManager::getTextureID(item.getTextureType()));
}

void DroppedItem::update(World& world) {
    transform.rotateY(2.0);

    glm::vec3 below = transform.position + glm::vec3(0.0f, -0.3f, 0.0f);
    if (world.getBlockAt(below) == nullptr) {
        verticalVelocity -= 0.01f;
        if (verticalVelocity < -0.12f) {
            verticalVelocity = -0.12f;
        }
        transform.translatePure(0.0f, verticalVelocity, 0.0f);
    } else if (verticalVelocity < 0.0f) {
        verticalVelocity = 0.0f;
    }
}

bool DroppedItem::canPickup(Player& player) const {
    return glm::distance(transform.position, player.getTransform()->position) <= pickupRadius;
}

const Item& DroppedItem::getItem() const {
    return item;
}
