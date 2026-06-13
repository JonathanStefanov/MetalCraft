#pragma once
#include "UIElement.h"
#include <string>
#include <functional>
#include <glm/glm.hpp>

class Font;

class Button : public UIElement {
public:
    Button(const std::string& text, float x, float y, float width, float height, Font* font, std::function<void()> onClick);

    void draw(Shader& shader) override;
    bool handleMouseMoved(double mouseX, double mouseY) override;
    bool handleMouseClicked(double mouseX, double mouseY, int button, int action) override;

private:
    std::string text;
    float x, y, width, height;
    Font* font;
    std::function<void()> onClick;
    bool isHovered = false;
};
