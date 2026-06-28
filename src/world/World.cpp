//
// Created by gaspa on 28/12/2022.
//

#include "World.h"
#include "../texture/manager/TextureManager.h"
#include "../objects/mesh/manager/MeshManager.h"
#include "glm/gtc/quaternion.hpp"

#include <utility>

const int FIELD_OF_VIEW = 100;

World::World(std::map<std::tuple<int, int, int>, std::tuple<int, MeshType, TextureType>> map, int length, int width,
             int depth) {
    worldBlocks = std::move(map);
    this->depth = depth;
    this->width = width;
    this->length = length;
}

void World::create() {


    for (auto &worldBlock: worldBlocks) {
        // print blockPos
        worldBlockInstances[worldBlock.first] = new GameObject(MeshManager::getMesh(std::get<1>(worldBlock.second)));
        worldBlockInstances[worldBlock.first]->setTexture(
                TextureManager::getTextureID(std::get<2>(worldBlock.second)));
        worldBlockInstances[worldBlock.first]->transform.setPosition(std::get<0>(worldBlock.first),
                                                                     std::get<2>(worldBlock.first),
                                                                     std::get<1>(worldBlock.first));
    }
}

void World::makeObjects(Shader &shader) {
    std::map<MTL::Texture*, GameObject *> bases = {};
    for (auto &worldBlockInstance: worldBlockInstances) {
        if (bases.find(worldBlockInstance.second->texture) == bases.end()) {
            bases[worldBlockInstance.second->texture] = worldBlockInstance.second;
            worldBlockInstance.second->makeObject(shader);
        } else {
            worldBlockInstance.second->makeObject(shader, bases[worldBlockInstance.second->texture]->renderer);
        }
    }

}

void World::draw(Shader &shader, glm::vec3 playerPosition) {
    for (auto &worldBlockInstance: worldBlockInstances) {
        if (glm::distance(playerPosition, worldBlockInstance.second->transform.getPosition()) < FIELD_OF_VIEW) {
            worldBlockInstance.second->draw(shader);
        }
    }
}

GameObject *World::getBlockAt(glm::vec3 &vec) {
    std::map<std::tuple<int, int, int>, GameObject *>::iterator findIterator;
    if (vec.y < 0) {
        findIterator = worldBlockInstances.find(
                std::make_tuple((int) vec.x, (int) vec.z, (int) vec.y - 1));

    } else {
        findIterator = worldBlockInstances.find(
                std::make_tuple((int) vec.x, (int) vec.z, (int) vec.y));

    }

    if (findIterator != worldBlockInstances.end()) {
        return findIterator->second;
    }
    return nullptr;
}

bool World::getBlockTextureTypeAt(glm::vec3 blockPos, TextureType& outTextureType) const {
    auto blockData = worldBlocks.find(std::make_tuple((int) blockPos.x, (int) blockPos.z, (int) blockPos.y));
    if (blockData == worldBlocks.end()) {
        return false;
    }

    outTextureType = std::get<2>(blockData->second);
    return true;
}

bool World::collides(IGameObject *object) {
    glm::vec3 position = object->getTransform()->position + glm::vec3(0.5f, 0, 0.5f);

    glm::vec3 corner = glm::vec3(object->collider.length / 2,
                                 0,
                                 object->collider.width / 2);


    std::vector<glm::vec3> collisionBox = {
            position,
            position - glm::vec3(corner.x, 0, corner.z),
            position + glm::vec3(corner.x, 0, corner.z),
            position + glm::vec3(-corner.x, 0, corner.z),
            position + glm::vec3(corner.x, 0, -corner.z),
            position - glm::vec3(corner.x/2, 0, corner.z),
            position + glm::vec3(corner.x, 0, corner.z/2),
            position + glm::vec3(-corner.x/2, 0, corner.z),
            position + glm::vec3(corner.x, 0, -corner.z/2),
            position + glm::vec3(corner.x, 0.05, corner.z),
            position + glm::vec3(-corner.x, 0.05, corner.z),
            position + glm::vec3(corner.x, 0.05, -corner.z),
    };

    bool didCollide = false;

    for (glm::vec3 &vec: collisionBox) {
        GameObject *blockAt = getBlockAt(vec);
        if (blockAt != nullptr && blockAt->texture != TextureManager::getTextureID(TextureType::WATER)) {
            didCollide = true;
            break;
        }
    }

    return didCollide;
}

Item World::removeBlock(glm::vec3 blockPos) {
    auto key = std::make_tuple((int) blockPos.x, (int) blockPos.z, (int) blockPos.y);
    auto blockData = worldBlocks.find(key);
    Item droppedItem;

    if (blockData != worldBlocks.end()) {
        droppedItem = Item::fromTextureType(std::get<2>(blockData->second), 1);
        worldBlocks.erase(blockData);
    }

    auto blockInstance = worldBlockInstances.find(key);
    if (blockInstance != worldBlockInstances.end()) {
        delete blockInstance->second;
        worldBlockInstances.erase(blockInstance);
    }

    return droppedItem;
}

float World::normalizeAngle(float angle) {
    while (angle < 0) angle += 360;
    while (angle >= 360) angle -= 360;
    return angle;
}

bool World::raycastBlocks(glm::vec3 startPos, glm::vec3 direction, float maxDistance, glm::vec3& outHitBlock, glm::vec3& outPreviousEmptyBlock) {
    float stepSize = 0.05f;
    glm::vec3 currentPos = startPos;
    glm::vec3 previousIntPos = glm::vec3(round(currentPos.x), round(currentPos.y), round(currentPos.z));

    for (float t = 0; t < maxDistance; t += stepSize) {
        currentPos = startPos + direction * t;
        glm::vec3 currentIntPos = glm::vec3(round(currentPos.x), round(currentPos.y), round(currentPos.z));
        
        // Check if there's a block at this coordinate
        std::tuple<int, int, int> key = std::make_tuple((int)currentIntPos.x, (int)currentIntPos.z, (int)currentIntPos.y);
        if (worldBlockInstances.find(key) != worldBlockInstances.end()) {
            outHitBlock = currentIntPos;
            outPreviousEmptyBlock = previousIntPos;
            return true;
        }
        
        // Only update previous empty block if the current int pos actually changed
        if (currentIntPos != previousIntPos) {
             previousIntPos = currentIntPos;
        }
    }
    return false;
}

bool World::addBlock(glm::vec3 blockPos, Shader &shader, TextureType textureType) {
    // insert into worldBlocks
    const std::tuple<int, int, int> &x = std::make_tuple((int) blockPos.x, (int) blockPos.z, (int) blockPos.y);
    if(!worldBlockInstances.count(x)){
        // No block at this position, can add the block at blockPos
        worldBlocks[x] = std::make_tuple(1, MeshType::BLOCK, textureType);
        worldBlockInstances[x] = new GameObject(MeshManager::getMesh(MeshType::BLOCK));
        worldBlockInstances[x]->setTexture(TextureManager::getTextureID(textureType));
        worldBlockInstances[x]->transform.setPosition((int) blockPos.x, (int) blockPos.y, (int) blockPos.z);

        // make object and draw
        worldBlockInstances[x]->makeObject(shader);

        return true;
    }

    return false;
}
