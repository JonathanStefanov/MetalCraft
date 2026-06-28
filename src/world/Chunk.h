//
// Chunk owns generated block data for a 16x16 world-space column area.
//

#ifndef OPENGLPROJECT_CHUNK_H
#define OPENGLPROJECT_CHUNK_H

#include <map>
#include "BlockData.h"

struct ChunkCoord {
    int x = 0;
    int z = 0;

    bool operator<(const ChunkCoord& other) const {
        if (x != other.x) {
            return x < other.x;
        }
        return z < other.z;
    }

    bool operator==(const ChunkCoord& other) const {
        return x == other.x && z == other.z;
    }
};

class Chunk {
public:
    explicit Chunk(ChunkCoord coord);

    ChunkCoord coord;
    std::map<BlockKey, BlockData> blocks;

    void setBlock(const BlockKey& key, BlockData data);
    void removeBlock(const BlockKey& key);
};

#endif //OPENGLPROJECT_CHUNK_H
