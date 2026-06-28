//
// Simple block payload used by generated chunks and player edits.
//

#ifndef OPENGLPROJECT_BLOCKDATA_H
#define OPENGLPROJECT_BLOCKDATA_H

#include <tuple>
#include "../objects/mesh/manager/MeshManager.h"
#include "../texture/manager/TextureManager.h"

using BlockKey = std::tuple<int, int, int>;

struct BlockData {
    MeshType meshType = MeshType::BLOCK;
    TextureType textureType = TextureType::DIRT;
};

#endif //OPENGLPROJECT_BLOCKDATA_H
