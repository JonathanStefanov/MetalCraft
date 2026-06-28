//
// Deterministic terrain generator for chunked Minecraft-style worlds.
//

#ifndef OPENGLPROJECT_WORLDGENERATOR_H
#define OPENGLPROJECT_WORLDGENERATOR_H

#include "Chunk.h"

class WorldGenerator {
public:
    explicit WorldGenerator(int seed);

    Chunk generateChunk(ChunkCoord coord) const;
    int getTerrainHeightAt(int x, int z) const;
    int getGroundHeightAt(int x, int z) const;
    bool isPondAt(int x, int z) const;

private:
    int seed;

    float hash01(int x, int z, int salt) const;
    float smoothNoise(float x, float z, float scale, int salt) const;
    bool shouldPlaceTreeAt(int x, int z) const;
    void addTreeBlocks(Chunk& chunk, int trunkX, int trunkZ) const;
    bool isInsideChunk(const Chunk& chunk, int x, int z) const;
};

#endif //OPENGLPROJECT_WORLDGENERATOR_H
