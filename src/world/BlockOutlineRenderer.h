#pragma once

#include "../utils/shader/shader/Shader.h"
#include "glm/vec3.hpp"
#include <vector>

namespace MTL {
    class Buffer;
}

class BlockOutlineRenderer {
public:
    BlockOutlineRenderer();
    ~BlockOutlineRenderer();

    void draw(Shader& shader, const glm::vec3& blockPosition);

    void drawCracks(Shader& shader, const glm::vec3& blockPosition, float progress);

private:
    MTL::Buffer* vertexBuffer = nullptr;
    std::vector<MTL::Buffer*> crackBuffers;
    std::vector<int> crackVertexCounts;
};
