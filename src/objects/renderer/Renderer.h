//
// Created by gaspa on 27/12/2022.
//

#ifndef OPENGLPROJECT_RENDERER_H
#define OPENGLPROJECT_RENDERER_H

#include "../../utils/shader/shader/Shader.h"
#include "../transform/Transform.h"
#include "../mesh/Mesh.h"

namespace MTL {
    class Buffer;
    class Texture;
    class RenderCommandEncoder;
}

class Renderer {
public:
    MTL::Buffer* vertexBuffer = nullptr;

    void makeObject(Shader &shader, Mesh *mesh, Transform &transform);

    void draw(Shader& shader, Transform &transform, Mesh *mesh, MTL::Texture* texture) const;

    void makeObject(Shader& shader, Mesh *pMesh, Transform& transform, Renderer& renderer);
};

#endif //OPENGLPROJECT_RENDERER_H
