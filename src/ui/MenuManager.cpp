#include "MenuManager.h"
#include "Button.h"
#include "Font.h"
#include "UIUtils.h"
#include "../game/GameState.h"
#include "../texture/manager/TextureManager.h"
#include "../utils/shader/shader/Shader.h"
#include <Metal/Metal.hpp>
#include <GLFW/glfw3.h>

extern GameState currentState;

MenuManager::MenuManager() : font(nullptr) {}

MenuManager::~MenuManager() {
    delete font;
}

void MenuManager::init(int screenWidth, int screenHeight, GLFWwindow* window) {
    font = new Font("resources/fonts/Minecraft.ttf", 64.0f);

    float btnWidth = 300.0f;
    float btnHeight = 80.0f;
    float startX = (screenWidth - btnWidth) / 2.0f;
    float startY = screenHeight / 2.0f - 100.0f;

    auto playBtn = std::make_unique<Button>("Play", startX, startY, btnWidth, btnHeight, font, [window]() {
        currentState = GameState::PLAYING;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    });

    auto quitBtn = std::make_unique<Button>("Quit", startX, startY + 120.0f, btnWidth, btnHeight, font, [window]() {
        glfwSetWindowShouldClose(window, true);
    });

    elements.push_back(std::move(playBtn));
    elements.push_back(std::move(quitBtn));
}

void MenuManager::draw(Shader& shader, int screenWidth, int screenHeight, MTL::Texture* bgTex, Shader* blurShader) {
    if (bgTex && blurShader) {
        blurShader->uiProjection = shader.uiProjection;
        blurShader->encoder = shader.encoder;
        
        // Bind the blur pipeline state!
        shader.encoder->setRenderPipelineState(blurShader->pipelineState);
        shader.encoder->setDepthStencilState(blurShader->depthStencilState);
        
        UIUtils::drawBlurredTexture(*blurShader, bgTex, 0, 0, screenWidth, screenHeight, screenWidth, screenHeight);
        
        // Restore the standard UI pipeline state for text and buttons
        shader.encoder->setRenderPipelineState(shader.pipelineState);
        shader.encoder->setDepthStencilState(shader.depthStencilState);
    }
    
    // Draw title
    float titleWidth = font->getTextWidth("Metalcraft");
    font->drawText(shader, "Metalcraft", (screenWidth - titleWidth) / 2.0f, 100.0f, glm::vec4(1.0f));
    
    // Draw subtitle
    float subWidth = font->getTextWidth("mac optimized");
    // we want a smaller font or just scale it? Font renderer doesn't support scaling yet, so we'll just draw it with the same font for now or add scale.
    // Let's just draw it with same font, maybe greyish color
    font->drawText(shader, "mac optimized", (screenWidth - subWidth) / 2.0f, 160.0f, glm::vec4(0.7f, 0.7f, 0.7f, 1.0f));

    for (auto& el : elements) {
        el->draw(shader);
    }
}

void MenuManager::handleMouseMoved(double x, double y) {
    for (auto& el : elements) {
        el->handleMouseMoved(x, y);
    }
}

void MenuManager::handleMouseClicked(double x, double y, int button, int action) {
    for (auto& el : elements) {
        if (el->handleMouseClicked(x, y, button, action)) {
            break;
        }
    }
}
