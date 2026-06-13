#pragma once
#include <glm/glm.hpp>

namespace MTL {
    class Texture;
}

class Shader;

class UIUtils {
public:
    static void drawRect(Shader& shader, float x, float y, float w, float h, const glm::vec4& color);
    static void drawTexture(Shader& shader, MTL::Texture* texture, float x, float y, float w, float h, const glm::vec4& color = glm::vec4(1.0f));
};
