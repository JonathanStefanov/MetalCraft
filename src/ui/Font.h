#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "stb_truetype.h"

namespace MTL {
    class Texture;
    class Buffer;
    class RenderCommandEncoder;
}

class Shader;

struct UIVertex {
    glm::vec2 position;
    glm::vec2 texCoord;
};

class Font {
public:
    Font(const std::string& fontPath, float size);
    ~Font();

    void drawText(Shader& shader, const std::string& text, float x, float y, const glm::vec4& color);
    
    float getTextWidth(const std::string& text);

private:
    stbtt_bakedchar cdata[96]; // ASCII 32..126 is 95 glyphs
    MTL::Texture* texture = nullptr;
    MTL::Buffer* vertexBuffer = nullptr;
    float fontSize;
};
