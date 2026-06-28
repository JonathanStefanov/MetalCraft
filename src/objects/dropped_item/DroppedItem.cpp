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

void DroppedItem::update(World& world, Player& player) {
    transform.rotateY(2.0);

    glm::vec3 playerPosition = player.getTransform()->position + glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 toPlayer = playerPosition - transform.position;
    float distanceToPlayer = glm::length(toPlayer);

    if (distanceToPlayer < attractionRadius && distanceToPlayer > 0.001f) {
        float speed = attractionSpeed * (1.0f - distanceToPlayer / attractionRadius) + 0.08f;
        transform.translatePure(toPlayer.x / distanceToPlayer * speed,
                                toPlayer.y / distanceToPlayer * speed,
                                toPlayer.z / distanceToPlayer * speed);
        return;
    }

    glm::vec3 below = transform.position + glm::vec3(0.0f, -0.3f, 0.0f);
    int blockY = below.y < 0 ? (int) below.y - 1 : (int) below.y;
    if (!world.isSolidBlockAt((int) below.x, blockY, (int) below.z)) {
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
