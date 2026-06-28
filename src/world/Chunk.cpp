#include "Chunk.h"

Chunk::Chunk(ChunkCoord coord) : coord(coord) {}

void Chunk::setBlock(const BlockKey& key, BlockData data) {
    blocks[key] = data;
}

void Chunk::removeBlock(const BlockKey& key) {
    blocks.erase(key);
}
