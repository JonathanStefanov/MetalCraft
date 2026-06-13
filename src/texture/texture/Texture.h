//
// Created by gaspa on 28/12/2022.
//

#ifndef OPENGLPROJECT_TEXTURE_H
#define OPENGLPROJECT_TEXTURE_H

#include <string>

namespace MTL { class Texture; }

class Texture {
public:
    MTL::Texture* texture = nullptr;

    explicit Texture(std::string path);
    ~Texture();

    MTL::Texture* getTexture() const;
};

#endif //OPENGLPROJECT_TEXTURE_H
