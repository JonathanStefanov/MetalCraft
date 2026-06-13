#include "Button.h"
#include "Font.h"
#include "UIUtils.h"
#include <GLFW/glfw3.h>

Button::Button(const std::string& text, float x, float y, float width, float height, Font* font, std::function<void()> onClick)
    : text(text), x(x), y(y), width(width), height(height), font(font), onClick(onClick) {}

void Button::draw(Shader& shader) {
    glm::vec4 bgColor = isHovered ? glm::vec4(0.4f, 0.4f, 0.4f, 1.0f) : glm::vec4(0.3f, 0.3f, 0.3f, 1.0f);
    UIUtils::drawRect(shader, x, y, width, height, bgColor);
    
    glm::vec4 color = isHovered ? glm::vec4(1.0f, 1.0f, 0.0f, 1.0f) : glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    
    float textWidth = font->getTextWidth(text);
    float textX = x + (width - textWidth) / 2.0f;
    float textY = y + height / 2.0f + 16.0f; // basic vertical alignment

    font->drawText(shader, text, textX, textY, color);
}

bool Button::handleMouseMoved(double mouseX, double mouseY) {
    isHovered = (mouseX >= x && mouseX <= x + width && mouseY >= y && mouseY <= y + height);
    return isHovered;
}

bool Button::handleMouseClicked(double mouseX, double mouseY, int button, int action) {
    if (isHovered && action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT) {
        if (onClick) onClick();
        return true;
    }
    return false;
}
