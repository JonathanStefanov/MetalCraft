#include "HUDManager.h"
#include "Font.h"
#include "UIUtils.h"
#include <string>

HUDManager::HUDManager() {
    font = new Font("resources/fonts/Minecraft.ttf", 24.0f);
}

HUDManager::~HUDManager() {
    delete font;
}

void HUDManager::draw(Shader& shader, int screenWidth, int screenHeight, const Inventory& inventory) {
    const float centerX = screenWidth / 2.0f;
    const float centerY = screenHeight / 2.0f;
    const float crosshairLength = 18.0f;
    const float crosshairThickness = 2.0f;
    const glm::vec4 crosshairColor = glm::vec4(0.95f, 0.95f, 0.95f, 0.9f);

    UIUtils::drawRect(shader, centerX - crosshairThickness / 2.0f, centerY - crosshairLength / 2.0f,
                      crosshairThickness, crosshairLength, crosshairColor);
    UIUtils::drawRect(shader, centerX - crosshairLength / 2.0f, centerY - crosshairThickness / 2.0f,
                      crosshairLength, crosshairThickness, crosshairColor);

    float slotSize = 60.0f;
    float padding = 5.0f;
    float totalWidth = (slotSize * 9) + (padding * 8);
    float startX = (screenWidth - totalWidth) / 2.0f;
    float startY = screenHeight - slotSize - 20.0f;

    for (int i = 0; i < Inventory::HOTBAR_SIZE; i++) {
        float x = startX + i * (slotSize + padding);
        
        // Draw selection highlight
        glm::vec4 bgColor = (i == inventory.selectedSlot) ? glm::vec4(0.9f, 0.9f, 0.9f, 0.9f) : glm::vec4(0.4f, 0.4f, 0.4f, 0.8f);
        UIUtils::drawRect(shader, x, startY, slotSize, slotSize, bgColor);
        
        // Draw inner slot background
        UIUtils::drawRect(shader, x + 4, startY + 4, slotSize - 8, slotSize - 8, glm::vec4(0.1f, 0.1f, 0.1f, 0.8f));

        // Draw item texture
        Item item = inventory.getHotbarItem(i);
        if (item.type != ItemType::NONE) {
            MTL::Texture* tex = TextureManager::getTextureID(item.getTextureType());
            if (tex) {
                UIUtils::drawTexture(shader, tex, x + 8, startY + 8, slotSize - 16, slotSize - 16);
            }

            if (item.quantity > 1 && font) {
                std::string quantityText = std::to_string(item.quantity);
                float textWidth = font->getTextWidth(quantityText);
                font->drawText(shader, quantityText, x + slotSize - textWidth - 6.0f, startY + slotSize - 8.0f,
                               glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
            }
        }
    }
}
