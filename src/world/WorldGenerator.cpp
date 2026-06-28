#include "WorldGenerator.h"
#include "World.h"

#include <glm/glm.hpp>
#include <cmath>
#include <cstdint>

WorldGenerator::WorldGenerator(int seed) : seed(seed) {}

float WorldGenerator::hash01(int x, int z, int salt) const {
    uint32_t h = (uint32_t) seed;
    h ^= (uint32_t) x * 374761393u;
    h = (h << 13u) ^ h;
    h ^= (uint32_t) z * 668265263u;
    h ^= (uint32_t) salt * 2246822519u;
    h ^= h >> 15u;
    h *= 2246822519u;
    h ^= h >> 13u;
    h *= 3266489917u;
    h ^= h >> 16u;
    return (float) h / (float) UINT32_MAX;
}

float WorldGenerator::smoothNoise(float x, float z, float scale, int salt) const {
    float scaledX = x / scale;
    float scaledZ = z / scale;
    int x0 = (int) std::floor(scaledX);
    int z0 = (int) std::floor(scaledZ);
    int x1 = x0 + 1;
    int z1 = z0 + 1;

    float tx = scaledX - (float) x0;
    float tz = scaledZ - (float) z0;
    tx = tx * tx * (3.0f - 2.0f * tx);
    tz = tz * tz * (3.0f - 2.0f * tz);

    float a = hash01(x0, z0, salt);
    float b = hash01(x1, z0, salt);
    float c = hash01(x0, z1, salt);
    float d = hash01(x1, z1, salt);

    float top = a + (b - a) * tx;
    float bottom = c + (d - c) * tx;
    return top + (bottom - top) * tz;
}

int WorldGenerator::getTerrainHeightAt(int x, int z) const {
    float continent = smoothNoise((float) x, (float) z, 72.0f, 1);
    float hills = smoothNoise((float) x, (float) z, 28.0f, 2);
    float detail = smoothNoise((float) x, (float) z, 10.0f, 3);
    float height = 1.0f + continent * 5.0f + hills * 4.0f + detail * 1.5f;

    if (isPondAt(x, z)) {
        return 0;
    }

    return (int) std::floor(height);
}

int WorldGenerator::getGroundHeightAt(int x, int z) const {
    return getTerrainHeightAt(x, z);
}

bool WorldGenerator::isPondAt(int x, int z) const {
    const int pondCellSize = 36;
    int cellX = World::floorDiv(x, pondCellSize);
    int cellZ = World::floorDiv(z, pondCellSize);

    for (int dz = -1; dz <= 1; ++dz) {
        for (int dx = -1; dx <= 1; ++dx) {
            int cx = cellX + dx;
            int cz = cellZ + dz;

            if (hash01(cx, cz, 50) > 0.32f) {
                continue;
            }

            int centerX = cx * pondCellSize + 8 + (int) (hash01(cx, cz, 51) * 20.0f);
            int centerZ = cz * pondCellSize + 8 + (int) (hash01(cx, cz, 52) * 20.0f);
            float radius = 4.0f + hash01(cx, cz, 53) * 5.0f;

            float distance = glm::distance(glm::vec2((float) x, (float) z),
                                           glm::vec2((float) centerX, (float) centerZ));
            if (distance <= radius) {
                return true;
            }
        }
    }

    return false;
}

bool WorldGenerator::shouldPlaceTreeAt(int x, int z) const {
    const int treeCellSize = 8;
    int cellX = World::floorDiv(x, treeCellSize);
    int cellZ = World::floorDiv(z, treeCellSize);

    if (hash01(cellX, cellZ, 80) > 0.42f) {
        return false;
    }

    int localX = (int) (hash01(cellX, cellZ, 81) * (float) treeCellSize);
    int localZ = (int) (hash01(cellX, cellZ, 82) * (float) treeCellSize);
    int candidateX = cellX * treeCellSize + localX;
    int candidateZ = cellZ * treeCellSize + localZ;

    if (x != candidateX || z != candidateZ) {
        return false;
    }

    if (isPondAt(x, z)) {
        return false;
    }

    for (int dz = -2; dz <= 2; ++dz) {
        for (int dx = -2; dx <= 2; ++dx) {
            if (isPondAt(x + dx, z + dz)) {
                return false;
            }
        }
    }

    return getGroundHeightAt(x, z) >= 1;
}

bool WorldGenerator::isInsideChunk(const Chunk& chunk, int x, int z) const {
    int minX = chunk.coord.x * World::CHUNK_SIZE;
    int minZ = chunk.coord.z * World::CHUNK_SIZE;
    return x >= minX && x < minX + World::CHUNK_SIZE &&
           z >= minZ && z < minZ + World::CHUNK_SIZE;
}

void WorldGenerator::addTreeBlocks(Chunk& chunk, int trunkX, int trunkZ) const {
    int groundY = getGroundHeightAt(trunkX, trunkZ);
    int trunkHeight = 4 + (int) (hash01(trunkX, trunkZ, 90) * 2.0f);

    for (int y = groundY + 1; y <= groundY + trunkHeight; ++y) {
        if (isInsideChunk(chunk, trunkX, trunkZ)) {
            chunk.setBlock(std::make_tuple(trunkX, trunkZ, y), {MeshType::BLOCK, TextureType::WOOD});
        }
    }

    int leafBase = groundY + trunkHeight - 1;
    for (int y = leafBase; y <= leafBase + 2; ++y) {
        int radius = y == leafBase + 2 ? 1 : 2;
        for (int dz = -radius; dz <= radius; ++dz) {
            for (int dx = -radius; dx <= radius; ++dx) {
                if (std::abs(dx) == 2 && std::abs(dz) == 2 && y == leafBase + 1) {
                    continue;
                }
                int x = trunkX + dx;
                int z = trunkZ + dz;
                if (isInsideChunk(chunk, x, z)) {
                    chunk.setBlock(std::make_tuple(x, z, y), {MeshType::BLOCK, TextureType::LEAF});
                }
            }
        }
    }
}

Chunk WorldGenerator::generateChunk(ChunkCoord coord) const {
    Chunk chunk(coord);
    int minX = coord.x * World::CHUNK_SIZE;
    int minZ = coord.z * World::CHUNK_SIZE;

    for (int x = minX; x < minX + World::CHUNK_SIZE; ++x) {
        for (int z = minZ; z < minZ + World::CHUNK_SIZE; ++z) {
            int groundY = getGroundHeightAt(x, z);
            int bottomY = groundY - 4;

            for (int y = bottomY; y < groundY; ++y) {
                chunk.setBlock(std::make_tuple(x, z, y), {MeshType::BLOCK, TextureType::DIRT});
            }

            if (isPondAt(x, z)) {
                chunk.setBlock(std::make_tuple(x, z, groundY), {MeshType::BLOCK, TextureType::DIRT});
                chunk.setBlock(std::make_tuple(x, z, 1), {MeshType::PLANE, TextureType::WATER});
            } else {
                chunk.setBlock(std::make_tuple(x, z, groundY), {MeshType::CUBEMAP, TextureType::GRASS});
            }
        }
    }

    for (int x = minX - 3; x < minX + World::CHUNK_SIZE + 3; ++x) {
        for (int z = minZ - 3; z < minZ + World::CHUNK_SIZE + 3; ++z) {
            if (shouldPlaceTreeAt(x, z)) {
                addTreeBlocks(chunk, x, z);
            }
        }
    }

    return chunk;
}
