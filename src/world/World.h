
//
// Created by gaspa on 28/12/2022.
//

#ifndef OPENGLPROJECT_WORLD_H
#define OPENGLPROJECT_WORLD_H


#include <map>
#include <set>
#include "glm/fwd.hpp"
#include "glm/detail/type_mat3x3.hpp"
#include "../objects/game_object/GameObject.h"
#include "../objects/mesh/manager/MeshManager.h"
#include "../texture/manager/TextureManager.h"
#include "../inventory/Item.h"
#include "Chunk.h"
#include "WorldGenerator.h"

class World {
public:
    World(std::map<std::tuple<int, int, int>, std::tuple<int, MeshType, TextureType>> map, int length, int width, int depth);
    explicit World(int seed);
    ~World();

    std::map<std::tuple<int, int, int>, GameObject*> worldBlockInstances;
public:
    static constexpr int CHUNK_SIZE = 16;
    static constexpr int LOAD_RADIUS = 4;

    int width{}, length{}, depth{};


    std::map<std::tuple<int, int, int>, std::tuple<int, MeshType, TextureType>> worldBlocks;



    void create();
    void updateLoadedChunks(glm::vec3 playerPosition, Shader& shader);
    int getTerrainHeightAt(int x, int z) const;

    float normalizeAngle(float angle);

    // Returns true if a block is hit within maxDistance.
    // outHitBlock: The integer coordinates of the solid block that was hit.
    // outPreviousEmptyBlock: The integer coordinates of the empty space right before the hit (useful for placing blocks).
    bool raycastBlocks(glm::vec3 startPos, glm::vec3 direction, float maxDistance, glm::vec3& outHitBlock, glm::vec3& outPreviousEmptyBlock);

    void draw(Shader& shader);

    Item removeBlock(glm::vec3 blockPos, Shader* shader = nullptr);


    void makeObjects(Shader &shader);

    bool collides(IGameObject* object);

    GameObject *getBlockAt(glm::vec3 &vec);

    bool isSolidBlockAt(int x, int y, int z) const;

    bool findStandingBlockTop(IGameObject* object, float previousY, float currentY, float& outTopY) const;

    bool getBlockTextureTypeAt(glm::vec3 blockPos, TextureType& outTextureType) const;

    void draw(Shader &shader, glm::vec3 playerPosition);

    bool addBlock(glm::vec3 blockPos, Shader &shader, TextureType textureType = TextureType::DIRT);

    static int floorDiv(int value, int divisor);
private:
    WorldGenerator generator;
    std::map<ChunkCoord, Chunk> loadedChunks;
    std::set<BlockKey> removedBlockOverrides;
    std::map<BlockKey, BlockData> placedBlockOverrides;

    ChunkCoord getChunkCoordForBlock(int x, int z) const;
    bool isChunkLoaded(ChunkCoord coord) const;
    void loadChunk(ChunkCoord coord, Shader* shader);
    void unloadChunk(ChunkCoord coord, Shader* shader);
    void addBlockObject(const BlockKey& key, const BlockData& data, Shader* shader);
    void removeBlockObject(const BlockKey& key);
    bool hasBlockAt(const BlockKey& key) const;
    bool isWaterBlockAt(const BlockKey& key) const;
    bool shouldRenderBlock(const BlockKey& key) const;
    void refreshBlockRenderState(const BlockKey& key, Shader* shader);
    void refreshBlockAndNeighbors(const BlockKey& key, Shader* shader);
    bool isKeyInsideChunk(const BlockKey& key, ChunkCoord coord) const;
};


#endif //OPENGLPROJECT_WORLD_H
