#pragma once

#include "../utils/shader/shader/Shader.h"
#include "glm/vec3.hpp"

namespace MTL {
    class Buffer;
}

class BlockOutlineRenderer {
public:
    BlockOutlineRenderer();
    ~BlockOutlineRenderer();

    void draw(Shader& shader, const glm::vec3& blockPosition);

private:
    MTL::Buffer* vertexBuffer = nullptr;
};
