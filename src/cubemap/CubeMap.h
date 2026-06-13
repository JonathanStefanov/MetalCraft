//
// Created by gaspa on 05/01/2023.
//

#ifndef OPENGLPROJECT_CUBEMAP_H
#define OPENGLPROJECT_CUBEMAP_H

#include <vector>
#include <map>
#include <string>
#include "../utils/shader/shader/Shader.h"

namespace MTL { class Texture; class Buffer; class RenderCommandEncoder; }

class CubeMap {
public:
    CubeMap(const std::string &pathSuffix, std::map<std::string, int> &faces); // Changed GLenum to int
    ~CubeMap();

    static std::vector<float> FacesCoords;

    MTL::Texture* texture = nullptr;
    MTL::Buffer* vertexBuffer = nullptr;

    void makeObject(Shader &shader);

    void draw(Shader &shader) const;
};

#endif //OPENGLPROJECT_CUBEMAP_H
