//
// Created by gaspa on 28/12/2022.
//

#include "World.h"
#include "../texture/manager/TextureManager.h"
#include "../objects/mesh/manager/MeshManager.h"
#include "glm/gtc/quaternion.hpp"

#include <cmath>
#include <utility>
#include <vector>

const int FIELD_OF_VIEW = 140;

World::World(std::map<std::tuple<int, int, int>, std::tuple<int, MeshType, TextureType>> map, int length, int width,
             int depth) : generator(1337) {
    worldBlocks = std::move(map);
    this->depth = depth;
    this->width = width;
    this->length = length;
}

World::World(int seed) : generator(seed) {}

World::~World() {
    for (auto& worldBlockInstance : worldBlockInstances) {
        delete worldBlockInstance.second;
    }
}

int World::floorDiv(int value, int divisor) {
    int quotient = value / divisor;
    int remainder = value % divisor;
    if (remainder != 0 && ((remainder < 0) != (divisor < 0))) {
        --quotient;
    }
    return quotient;
}

void World::create() {
    std::vector<BlockKey> keys;
    for (const auto& worldBlock : worldBlocks) {
        keys.push_back(worldBlock.first);
    }

    for (const BlockKey& key : keys) {
        refreshBlockRenderState(key, nullptr);
    }
}

ChunkCoord World::getChunkCoordForBlock(int x, int z) const {
    return {floorDiv(x, CHUNK_SIZE), floorDiv(z, CHUNK_SIZE)};
}

bool World::isChunkLoaded(ChunkCoord coord) const {
    return loadedChunks.find(coord) != loadedChunks.end();
}

bool World::isKeyInsideChunk(const BlockKey& key, ChunkCoord coord) const {
    int x = std::get<0>(key);
    int z = std::get<1>(key);
    int minX = coord.x * CHUNK_SIZE;
    int minZ = coord.z * CHUNK_SIZE;

    return x >= minX && x < minX + CHUNK_SIZE &&
           z >= minZ && z < minZ + CHUNK_SIZE;
}

void World::addBlockObject(const BlockKey& key, const BlockData& data, Shader* shader) {
    if (worldBlockInstances.find(key) != worldBlockInstances.end()) {
        return;
    }

    auto* block = new GameObject(MeshManager::getMesh(data.meshType));
    block->setTexture(TextureManager::getTextureID(data.textureType));
    block->transform.setPosition(std::get<0>(key), std::get<2>(key), std::get<1>(key));

    if (shader != nullptr) {
        block->makeObject(*shader);
    }

    worldBlockInstances[key] = block;
}

void World::removeBlockObject(const BlockKey& key) {
    auto blockInstance = worldBlockInstances.find(key);
    if (blockInstance != worldBlockInstances.end()) {
        delete blockInstance->second;
        worldBlockInstances.erase(blockInstance);
    }
}

bool World::hasBlockAt(const BlockKey& key) const {
    return worldBlocks.find(key) != worldBlocks.end();
}

bool World::isWaterBlockAt(const BlockKey& key) const {
    auto blockData = worldBlocks.find(key);
    if (blockData == worldBlocks.end()) {
        return false;
    }

    return std::get<2>(blockData->second) == TextureType::WATER;
}

bool World::shouldRenderBlock(const BlockKey& key) const {
    auto blockData = worldBlocks.find(key);
    if (blockData == worldBlocks.end()) {
        return false;
    }

    TextureType textureType = std::get<2>(blockData->second);
    if (textureType == TextureType::WATER) {
        return true;
    }

    int x = std::get<0>(key);
    int z = std::get<1>(key);
    int y = std::get<2>(key);
    std::vector<BlockKey> neighbors = {
            std::make_tuple(x + 1, z, y),
            std::make_tuple(x - 1, z, y),
            std::make_tuple(x, z + 1, y),
            std::make_tuple(x, z - 1, y),
            std::make_tuple(x, z, y + 1),
            std::make_tuple(x, z, y - 1),
    };

    for (const BlockKey& neighbor : neighbors) {
        if (!hasBlockAt(neighbor) || isWaterBlockAt(neighbor)) {
            return true;
        }
    }

    return false;
}

void World::refreshBlockRenderState(const BlockKey& key, Shader* shader) {
    auto blockData = worldBlocks.find(key);
    if (blockData == worldBlocks.end()) {
        removeBlockObject(key);
        return;
    }

    if (!shouldRenderBlock(key)) {
        removeBlockObject(key);
        return;
    }

    addBlockObject(key, {std::get<1>(blockData->second), std::get<2>(blockData->second)}, shader);
}

void World::refreshBlockAndNeighbors(const BlockKey& key, Shader* shader) {
    int x = std::get<0>(key);
    int z = std::get<1>(key);
    int y = std::get<2>(key);
    std::vector<BlockKey> blocksToRefresh = {
            key,
            std::make_tuple(x + 1, z, y),
            std::make_tuple(x - 1, z, y),
            std::make_tuple(x, z + 1, y),
            std::make_tuple(x, z - 1, y),
            std::make_tuple(x, z, y + 1),
            std::make_tuple(x, z, y - 1),
    };

    for (const BlockKey& blockToRefresh : blocksToRefresh) {
        refreshBlockRenderState(blockToRefresh, shader);
    }
}

void World::loadChunk(ChunkCoord coord, Shader* shader) {
    if (isChunkLoaded(coord)) {
        return;
    }

    Chunk chunk = generator.generateChunk(coord);

    for (const BlockKey& removedBlock : removedBlockOverrides) {
        if (isKeyInsideChunk(removedBlock, coord)) {
            chunk.removeBlock(removedBlock);
        }
    }

    for (const auto& placedBlock : placedBlockOverrides) {
        if (isKeyInsideChunk(placedBlock.first, coord)) {
            chunk.setBlock(placedBlock.first, placedBlock.second);
        }
    }

    auto insertedChunk = loadedChunks.insert(std::make_pair(coord, chunk));

    for (const auto& block : insertedChunk.first->second.blocks) {
        worldBlocks[block.first] = std::make_tuple(1, block.second.meshType, block.second.textureType);
    }

    for (const auto& block : insertedChunk.first->second.blocks) {
        refreshBlockAndNeighbors(block.first, shader);
    }
}

void World::unloadChunk(ChunkCoord coord, Shader* shader) {
    auto chunkIterator = loadedChunks.find(coord);
    if (chunkIterator == loadedChunks.end()) {
        return;
    }

    std::vector<BlockKey> unloadedKeys;
    for (const auto& block : chunkIterator->second.blocks) {
        unloadedKeys.push_back(block.first);
        removeBlockObject(block.first);
        worldBlocks.erase(block.first);
    }

    loadedChunks.erase(chunkIterator);

    for (const BlockKey& key : unloadedKeys) {
        refreshBlockAndNeighbors(key, shader);
    }
}

void World::updateLoadedChunks(glm::vec3 playerPosition, Shader& shader) {
    int playerX = (int) std::floor(playerPosition.x);
    int playerZ = (int) std::floor(playerPosition.z);
    ChunkCoord center = getChunkCoordForBlock(playerX, playerZ);

    std::set<ChunkCoord> chunksToKeep;
    for (int dz = -LOAD_RADIUS; dz <= LOAD_RADIUS; ++dz) {
        for (int dx = -LOAD_RADIUS; dx <= LOAD_RADIUS; ++dx) {
            ChunkCoord coord{center.x + dx, center.z + dz};
            chunksToKeep.insert(coord);
            loadChunk(coord, &shader);
        }
    }

    std::vector<ChunkCoord> chunksToUnload;
    for (const auto& loadedChunk : loadedChunks) {
        if (chunksToKeep.find(loadedChunk.first) == chunksToKeep.end()) {
            chunksToUnload.push_back(loadedChunk.first);
        }
    }

    for (const ChunkCoord& coord : chunksToUnload) {
        unloadChunk(coord, &shader);
    }
}

int World::getTerrainHeightAt(int x, int z) const {
    return generator.getTerrainHeightAt(x, z);
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
        int blockY = vec.y < 0 ? (int) vec.y - 1 : (int) vec.y;
        BlockKey key = std::make_tuple((int) vec.x, (int) vec.z, blockY);
        if (hasBlockAt(key) && !isWaterBlockAt(key)) {
            didCollide = true;
            break;
        }
    }

    return didCollide;
}

bool World::isSolidBlockAt(int x, int y, int z) const {
    BlockKey key = std::make_tuple(x, z, y);
    if (!hasBlockAt(key)) {
        return false;
    }

    return !isWaterBlockAt(key);
}

bool World::findStandingBlockTop(IGameObject* object, float previousY, float currentY, float& outTopY) const {
    glm::vec3 position = object->getTransform()->position;
    float halfLength = object->collider.length / 2.0f;
    float halfWidth = object->collider.width / 2.0f;

    std::vector<glm::vec2> samples = {
            {position.x, position.z},
            {position.x - halfLength, position.z - halfWidth},
            {position.x + halfLength, position.z - halfWidth},
            {position.x - halfLength, position.z + halfWidth},
            {position.x + halfLength, position.z + halfWidth},
    };

    bool foundTop = false;
    float bestTop = currentY;

    int minY = (int) std::floor(currentY) - 1;
    int maxY = (int) std::floor(previousY);

    for (const glm::vec2& sample : samples) {
        int blockX = (int) std::round(sample.x);
        int blockZ = (int) std::round(sample.y);

        for (int blockY = maxY; blockY >= minY; --blockY) {
            float blockTop = (float) blockY + 1.0f;
            if (previousY >= blockTop && currentY <= blockTop && isSolidBlockAt(blockX, blockY, blockZ)) {
                if (!foundTop || blockTop > bestTop) {
                    bestTop = blockTop;
                    foundTop = true;
                }
            }
        }
    }

    if (foundTop) {
        outTopY = bestTop;
    }

    return foundTop;
}

Item World::removeBlock(glm::vec3 blockPos, Shader* shader) {
    auto key = std::make_tuple((int) blockPos.x, (int) blockPos.z, (int) blockPos.y);
    auto blockData = worldBlocks.find(key);
    Item droppedItem;

    if (blockData != worldBlocks.end()) {
        droppedItem = Item::fromTextureType(std::get<2>(blockData->second), 1);
        worldBlocks.erase(blockData);
    }

    placedBlockOverrides.erase(key);
    removedBlockOverrides.insert(key);

    ChunkCoord chunkCoord = getChunkCoordForBlock(std::get<0>(key), std::get<1>(key));
    auto chunk = loadedChunks.find(chunkCoord);
    if (chunk != loadedChunks.end()) {
        chunk->second.removeBlock(key);
    }

    auto blockInstance = worldBlockInstances.find(key);
    if (blockInstance != worldBlockInstances.end()) {
        delete blockInstance->second;
        worldBlockInstances.erase(blockInstance);
    }

    refreshBlockAndNeighbors(key, shader);

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
        if (worldBlocks.find(key) != worldBlocks.end()) {
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
    if(!worldBlocks.count(x)){
        // No block at this position, can add the block at blockPos
        BlockData blockData{MeshType::BLOCK, textureType};
        removedBlockOverrides.erase(x);
        placedBlockOverrides[x] = blockData;
        worldBlocks[x] = std::make_tuple(1, blockData.meshType, blockData.textureType);

        ChunkCoord chunkCoord = getChunkCoordForBlock(std::get<0>(x), std::get<1>(x));
        auto chunk = loadedChunks.find(chunkCoord);
        if (chunk != loadedChunks.end()) {
            chunk->second.setBlock(x, blockData);
        }

        refreshBlockAndNeighbors(x, &shader);

        return true;
    }

    return false;
}
